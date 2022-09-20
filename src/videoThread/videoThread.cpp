
#include <thread>

#include "videoThread.h"
#include "decode.h"
#include "videoCall.h"
#include "audioThread.h"


VideoThread::VideoThread() {
	decode = new Decode;

	frame = av_frame_alloc();
}

int VideoThread::open(AVCodecParameters* parameters, VideoCall* videoCall) {
	int ret;
	if (!decode) {
		fprintf(stderr, "please init decode!\n");
		return -1;
	}
	if (!parameters) {
		fprintf(stderr, "the parameters is empty!\n");
		return -1;
	}


	this->call = videoCall;
	if (call) {
		call->init(parameters->width, parameters->height);
	}

	/*这里open会重新分配avcodec_alloc_context3*/
	ret = decode->open(parameters);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

void VideoThread::run() {
	int ret = 0;
	while (!isExit) {
		if (!decode) {
			fprintf(stderr, "please initialize\n");
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
			fprintf(stderr, "video发给解码器失败\n");
			continue;
		}
		lock.unlock();

		/*
		 * 如果把这一块锁住，有可能，里面的那个while循环是跳不出去，
		 * 因为如果视频pts小于音频pts的话，就要等待音频pts追上来，
		 * 这里就一直循环卡住等待音频，又因为这块锁住了，音频那边又push不进去数据，
		 * 就会造成卡死
		 * */
		if (currSerial == serial) {
			display = false;
			while (ret >= 0) {
				ret = decode->receiver(frame);
				if (ret < 0) {

					if (ret == AVERROR_EOF) {
						printf("video AVERROR_EOF\n");
						decode->clear();
						break;
					}
					/*当前不接受输出，必须重新发送*/
					if (ret == AVERROR(EAGAIN)) {
						break;
					}

					char errbuf[AV_ERROR_MAX_STRING_SIZE]{ 0 };
					av_make_error_string(errbuf, AV_ERROR_MAX_STRING_SIZE, ret);
					fprintf(stderr, "Error during decoding (%s)\n", errbuf);
					break;
				}
				/*这里要判断是否是连续的，连续的才去等待*/
				while (currSerial == serial) {
					/*视频pts大于音频pts的话就等待音频追上来*/
					if (frame->pts > at->pts && !isPause) {
						std::this_thread::sleep_for(std::chrono::milliseconds(3));
						continue;
					} else {
						break;
					}
				}

				this->call->repaint(frame);
				av_frame_unref(frame);

				display = true;
			}
		}
	}
}

int VideoThread::clear() {
	mux.lock();

	/*清楚codec内部缓存
	但是这里有个问题，有可能这个线程清理的时候，另一个线程正在解码
	*/
	/*这里clear的时候解码器有可能在读取缓存里的帧，就会造成音频跳转到了，100，
	 * 而视频是之前缓存的500，里面的那个循环就会一直等待音频追上来*/
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

int VideoThread::push(MyAVPacket* pkt, int CurrSerial, bool& exitFlag) {
	if (!pkt) {
		return -1;
	}

	while (!exitFlag) {
		std::unique_lock<std::mutex> lock(mux);
		/*这里需要被锁住，因为如果list还有切换之前的packet的话，
		上面的那个whlie循环 while (!list.empty()) 就有可能把之前的packet全部销毁掉
		导致取不到数据，这里更新serial的话，能保证走到while循环那里，
		list里面有一个切换之后的packet数据*/
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

void VideoThread::setPause(bool state) {
	isPause = state;
}

VideoThread::~VideoThread() {
	isExit = true;
	/* 等待线程退出 */
	wait();

	clear();

	if (decode) {
		delete decode;
		decode = nullptr;
	}
	av_frame_free(&frame);


}





