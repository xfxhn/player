
#include <thread>
#include "demuxThread.h"
#include "demux.h"
#include "audioThread.h"
#include "videoThread.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

DemuxThread::DemuxThread() {
	pkt = av_packet_alloc();


	demux = new Demux;
	at = new AudioThread;
	vt = new VideoThread;
}

int DemuxThread::open(const char* url, VideoCall* call) {
	if (!demux || !at || !vt) {
		return -1;
	}

	int ret;

	ret = demux->open(url);
	if (ret < 0) {
		fprintf(stderr, "demux failed");
		return ret;
	}

	AVCodecParameters* audioPara = demux->getAudioParameters();
	ret = at->open(audioPara, demux->getAudioTimeBase());
	if (ret < 0) {
		fprintf(stderr, "open audio thread failed");
		return ret;
	}
	avcodec_parameters_free(&audioPara);


	AVCodecParameters* videoPara = demux->getVideoParameters();
	ret = vt->open(videoPara, call);
	if (ret < 0) {
		fprintf(stderr, "open video thread failed");
		return ret;
	}
	avcodec_parameters_free(&videoPara);

	return 0;
}

int DemuxThread::start() {

	if (demux) {
		QThread::start();
	}

	if (vt) {
		vt->start();
	}
	if (at) {
		at->start();
	}
	if (vt && at) {
		vt->at = at;
	}
	return 0;
}

int64_t DemuxThread::getCurrentMillisecond() {

	if (at) {
		return at->pts;
	}
	return 0;
}

int64_t DemuxThread::getDuration() {
	if (demux) {
		return demux->getDuration();
	}
	return 0;
}

void DemuxThread::run() {

	int ret;
	while (!isExit) {
		if (!demux || !at || !vt) {
			//av_read_pause(demux->fmtCtx);
			fprintf(stderr, "please initialize！");
			return;
		}

		mux.lock();
		ret = demux->readPacket(pkt);
		MyAVPacket* aaa = new MyAVPacket;
		aaa->serial = serial;
		aaa->pkt = *pkt;
		mux.unlock();

		if (ret == -1) {
			fprintf(stderr, "read packet error\n");
			break;
		} else if (ret == 2) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		} else if (ret == 1) {
			fprintf(stdout, "read finish ok\n");

			/*刷新解码器*/
			/*	pkt->data = nullptr;
				pkt->size = 0;

				vt->push(aaa, serial);
				at->push(aaa, serial);*/
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
			/*别退出，有可能跳转播放*/
			//break;
		}

		if (demux->isVideo(pkt)) {
			vt->push(aaa, serial, isExit);
		} else {
			at->push(aaa, serial, isExit);
		}


	}
}

int DemuxThread::seek(double pos) {
	int ret = 0;

	if (demux) {
		mux.lock();
		clear();
		serial = serial + 1;

		ret = demux->seek(pos);
		mux.unlock();
		if (ret < 0) {
			fprintf(stderr, "seek failed\n");
		}
	}
	return ret;
}

int DemuxThread::clear() {

	if (demux)demux->clear();
	if (vt)vt->clear();
	if (at)at->clear();

	return 0;
}

void DemuxThread::setPause(bool state) {
	if (at) {
		at->setPause(state);
	}

	if (vt) {
		vt->setPause(state);
	}
}


DemuxThread::~DemuxThread() {

	isExit = true;
	/*等待线程执行结束*/
	this->wait();

	delete demux;
	demux = nullptr;


	delete vt;
	vt = nullptr;

	delete at;
	at = nullptr;

	/*
	这里有可能这个packet已经push到list里面了，
	然后这里释放了会调用av_buffer_unref，
	然后音频线程再次释放,就会释放同一块内存
	这里初始化为null，只释放packet这个结构体
	*/
	av_init_packet(pkt);
	av_packet_free(&pkt);

}

int DemuxThread::setSpeed(const char* speed) {
	at->setSpeed(speed);

	return 0;
}






