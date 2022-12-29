#ifndef PLAYER_VIDEOTHREAD_H
#define PLAYER_VIDEOTHREAD_H

#include <list>
#include <mutex>
#include <condition_variable>
#include "QThread"

extern "C" {
#include "libavcodec/avcodec.h"
}

class Decode;

struct AVCodecParameters;

class VideoCall;

class AudioThread;


struct MyAVPacket {
	AVPacket pkt;
	int serial;
};

class VideoThread : public QThread {
public:
	VideoThread();

	int open(AVCodecParameters* parameters, VideoCall* call);

	void run() override;

	int push(MyAVPacket* pkt, int CurrSerial, bool& exitFlag);


	int clear();

	~VideoThread() override;

	bool isPause{ false };

	void setPause(bool state);

public:
	AudioThread* at{ nullptr };
private:
	int currSerial{ 0 };
	int serial{ 0 };
	std::list<MyAVPacket*> list;
	std::mutex mux;
	std::condition_variable cv;
	Decode* decode{ nullptr };

	bool isExit{ false };
	/*列表最多缓存这么多*/
	static constexpr int maxSize{ 1000 };

	AVFrame* frame{ nullptr };

	VideoCall* call{ nullptr };


	bool display{ false };
};


#endif //PLAYER_VIDEOTHREAD_H
