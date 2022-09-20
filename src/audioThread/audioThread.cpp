
#include <thread>
#include "audioThread.h"
#include "decode.h"
#include "audioPlay.h"
#include "resample.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
}

struct MyAVPacket {
	AVPacket pkt;
	int serial;
};

AudioThread::AudioThread() {
	frame = av_frame_alloc();

	decode = new Decode;
	rs = new Resample;
	audio = AudioPlay::get();

}

int AudioThread::open(AVCodecParameters* parameters, const AVRational& base) {


	if (!decode || !rs || !audio) {
		return -1;
	}
	this->timeBase = base;
	int ret;
	if (!parameters) {
		fprintf(stderr, "the parameters is empty!\n");
		return -1;
	}

	ret = decode->open(parameters);
	if (ret < 0) {
		return ret;
	}

	ret = rs->open(parameters);
	if (ret < 0) {
		return ret;
	}
	sampleRate = parameters->sample_rate;
	format = parameters->format;
	channel_layout = parameters->channel_layout;

	audio->channels = parameters->channels;
	audio->sampleRate = sampleRate;
	ret = audio->open();
	if (ret < 0) {
		fprintf(stderr, "open audio player failed");
		return ret;
	}

	ret = av_samples_alloc_array_and_samples(&pcmData, nullptr, parameters->channels,
											 parameters->frame_size, AV_SAMPLE_FMT_S16, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate destination samples\n");
		return ret;
	}


	ret = init_filter_graph("1.0");
	if (ret < 0) {
		fprintf(stderr, "init filter failed\n");
		return ret;

	}
	return 0;
}

void AudioThread::run() {
	int ret = 0;
	while (!isExit) {

		if (!decode || !rs) {
			fprintf(stderr, "audio thread please initialized\n");
			return;
		}

		if (isPause && currSerial == serial && display) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}


		std::unique_lock<std::mutex> lock(mux);

		cv.wait(lock, [&] {
			return !list.empty();
				});


		MyAVPacket* pkt;
		while (!list.empty()) {
			pkt = list.front();
			currSerial = pkt->serial;
			if (pkt->serial == serial) {
				ret = decode->send(&pkt->pkt);
				/* 不管是否错误都给丢弃掉这个packet */
				av_packet_unref(&pkt->pkt);
				list.pop_front();
				delete pkt;
				break;
			} else {
				av_packet_unref(&pkt->pkt);
				list.pop_front();
				delete pkt;
			}
		}
		if (ret < 0) {
			fprintf(stderr, "发给解码器失败\n");
			continue;
		}
		lock.unlock();

		if (currSerial == serial) {
			display = false;
			while (ret >= 0) {

				ret = decode->receiver(frame);

				if (ret < 0) {
					if (ret == AVERROR_EOF) {
						printf("audio AVERROR_EOF\n");
						decode->clear();
						//return;
						break;
					}
					if (ret == AVERROR(EAGAIN)) {
						break;
					}

					char errbuf[AV_ERROR_MAX_STRING_SIZE]{ 0 };
					av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, ret);
					fprintf(stderr, "Error during decoding (%s)\n", errbuf);
					break;
				}

				/*计算当前音频播放到了哪里ms*/
				pts = frame->pts;// -audio->getNoPlayPts();
				if (speedFlag) {
					speedFlag = false;
					int ret = init_filter_graph(speed);
					if (ret < 0) {
						av_log(nullptr, AV_LOG_ERROR, "init filter failed.\n");
						break;
					}
				}




				/*将帧发送到过滤器的输入端。*/
				/*这里面做了 ret = av_frame_move_ref;*/
				ret = av_buffersrc_add_frame(src, frame);
				if (ret < 0) {
					fprintf(stderr, "Error submitting the frame to the filtergraph:");
					av_frame_unref(frame);
					break;
				}
				while (true) {
					ret = av_buffersink_get_frame(out, frame);
					if (ret < 0) {
						if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
							break;
						char errbuf[AV_ERROR_MAX_STRING_SIZE]{ 0 };
						av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, ret);
						fprintf(stderr, "Error during decoding (%s)\n", errbuf);
						break;
					}


					int bufSize = av_samples_get_buffer_size(nullptr, frame->channels,
															 frame->nb_samples,
															 AVSampleFormat(frame->format), 1);
					while (bufSize > 0) {
						/* 等待播放完缓冲数据，然后写入 */
						if (audio->getFree() >= bufSize) {
							audio->write(frame->data[0], bufSize);
							break;
						}
						/*减少循环次数*/
						std::this_thread::sleep_for(std::chrono::milliseconds(5));
					}

					av_frame_unref(frame);
					display = true;
				}

				//av_frame_unref(frame);
				//rs->resample(frame, pcmData);

				/*int bufSize = av_samples_get_buffer_size(nullptr, frame->channels,
														 frame->nb_samples, AVSampleFormat(frame->format), 1);
				if (frame->format != AV_SAMPLE_FMT_S16) {
					bufSize = rs->resample(frame, pcmData);
				}

				while (bufSize > 0) {
					*//* 等待播放完缓冲数据，然后写入 *//*
					if (audio->getFree() >= bufSize) {
						audio->write(pcmData[0], bufSize);
						break;
					}
					*//*减少循环次数*//*
					std::this_thread::sleep_for(std::chrono::milliseconds(5));
				}
				av_frame_unref(frame);
				display = true;*/
			}
		}

	}
}
int AudioThread::setSpeed(const char* _speed) {
	if (_speed == speed) {
		return 0;
	}
	speed = _speed;
	speedFlag = true;
	return 0;
}


int AudioThread::init_filter_graph(const char* value) {
	/*abuffer滤镜和abuffersink滤镜是两个特殊的音频滤镜，分别用于音频滤镜链的输入端和输出端*/
	int ret = 0;


	//初始化AVFilterGraph
	avfilter_graph_free(&filter_graph);
	filter_graph = avfilter_graph_alloc();
	if (!filter_graph) {
		fprintf(stderr, "init filter failed\n");
		return -1;
	}




	//abuffer用于接收输入frame,形成待处理的数据缓存
	const AVFilter* abuffer = avfilter_get_by_name("abuffer");
	if (!abuffer) {
		fprintf(stderr, "Could not find the abuffer filter.\n");
		return AVERROR_FILTER_NOT_FOUND;
	}

	char args[512]{ 0 };
	snprintf(args, sizeof(args),
			 "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRId64,
			 timeBase.num, timeBase.den, sampleRate, av_get_sample_fmt_name(AVSampleFormat(format)), channel_layout);


	AVFilterContext* abuffer_ctx;
	ret = avfilter_graph_create_filter(&abuffer_ctx, abuffer, "src",
									   args, nullptr, filter_graph);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Cannot create audio buffer source\n");
		return ret;
	}


	const AVFilter* atempo = avfilter_get_by_name("atempo");
	if (!atempo) {
		fprintf(stderr, "Could not find the atempo filter.\n");
		return AVERROR_FILTER_NOT_FOUND;
	}
	AVFilterContext* atempo_ctx = avfilter_graph_alloc_filter(filter_graph, atempo, "atempo");
	if (!atempo_ctx) {
		fprintf(stderr, "Could not allocate the atempo instance.\n");
		return AVERROR(ENOMEM);
	}
	//这里采用av_dict_set设置参数
	AVDictionary* info = nullptr;
	av_dict_set(&info, "tempo", value, 0);//这里传入外部参数，可以动态修改
	ret = avfilter_init_dict(atempo_ctx, &info);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Could not initialize the atempo filter.\n");
		return ret;
	}


	const AVFilter* aformat = avfilter_get_by_name("aformat");
	if (!aformat) {
		fprintf(stderr, "Could not find the aformat filter.\n");
		return AVERROR_FILTER_NOT_FOUND;
	}

	AVFilterContext* aformat_ctx = avfilter_graph_alloc_filter(filter_graph, aformat, "aformat");
	if (!aformat_ctx) {
		fprintf(stderr, "Could not allocate the aformat instance.\n");
		return AVERROR(ENOMEM);
	}

	char options_str[1024]{ 0 };
	snprintf(options_str, sizeof(options_str),
			 "sample_fmts=%s:sample_rates=%d:channel_layouts=0x%" PRId64,
			 "s16", sampleRate, channel_layout);
	ret = avfilter_init_str(aformat_ctx, options_str);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Could not initialize the aformat filter.\n");
		return ret;
	}



	//它将用于从graph中获得过滤后的数据。
	const AVFilter* abuffersink = avfilter_get_by_name("abuffersink");
	if (!abuffersink) {
		fprintf(stderr, "Could not find the abuffersink filter.\n");
		return AVERROR_FILTER_NOT_FOUND;
	}
	AVFilterContext* abuffersink_ctx = avfilter_graph_alloc_filter(filter_graph, abuffersink, "sink");
	if (!abuffersink_ctx) {
		fprintf(stderr, "Could not allocate the abuffersink instance.\n");
		return AVERROR(ENOMEM);
	}
	/*此过滤器不接受任何选项。*/
	ret = avfilter_init_str(abuffersink_ctx, nullptr);
	if (ret < 0) {//无需参数
		fprintf(stderr, "Could not initialize the abuffersink instance.\n");
		return -1;
	}

	//链接各个filter上下文
	ret = avfilter_link(abuffer_ctx, 0, atempo_ctx, 0);
	if (ret >= 0)
		ret = avfilter_link(atempo_ctx, 0, aformat_ctx, 0);
	if (ret >= 0)
		ret = avfilter_link(aformat_ctx, 0, abuffersink_ctx, 0);
	if (ret < 0) {
		fprintf(stderr, "Error connecting filters\n");
		return ret;
	}


	ret = avfilter_graph_config(filter_graph, nullptr);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Error configuring the filter graph\n");
		return ret;
	}
	src = abuffer_ctx;
	out = abuffersink_ctx;
	return 0;
}

void AudioThread::setPause(bool state) {
	isPause = state;
	//audio->setPause(state);
}

int AudioThread::push(MyAVPacket* pkt, int CurrSerial, bool& exitFlag) {
	/*执行这个函数的永远都是demux线程*/
	if (!pkt) {
		return -1;
	}

	while (!exitFlag) {
		std::unique_lock<std::mutex> lock(mux);

		serial = CurrSerial;
		if (list.size() < maxSize) {
			list.push_back(pkt);
			lock.unlock();
			cv.notify_one();
			break;
		} else {
			/*数据满了，等待消耗*/
			lock.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}
	return 0;
}


int AudioThread::clear() {
	mux.lock();
	/*if (audio) {
		audio->clear();
	}*/


	/*这里clear的时候解码器有可能在读取缓存里的帧*/
	if (decode) decode->clear();
	while (!list.empty()) {
		MyAVPacket* pkt = list.front();
		list.pop_front();
		av_packet_unref(&pkt->pkt);
		delete pkt;
	}
	mux.unlock();

	return 0;
}

AudioThread::~AudioThread() {
	isExit = true;

	/* 等待线程退出 */
	wait();

	avfilter_graph_free(&filter_graph);

	if (pcmData)
		av_freep(&pcmData[0]);
	av_freep(&pcmData);
	clear();

	av_frame_free(&frame);
	if (decode) {
		delete decode;
		decode = nullptr;
	}
	if (rs) {
		delete rs;
		rs = nullptr;
	}

}



