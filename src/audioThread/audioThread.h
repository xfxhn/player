#ifndef PLAYER_AUDIOTHREAD_H
#define PLAYER_AUDIOTHREAD_H

#include <mutex>
#include <condition_variable>
#include <list>
#include "QThread"

extern "C" {
#include "libavcodec/avcodec.h"
}
struct AVCodecParameters;

class Decode;

class AudioPlay;

class Resample;

struct MyAVPacket;

struct AVFilterGraph;
struct AVFilterContext;

class AudioThread : public QThread {

public:
	AudioThread();

	int open(AVCodecParameters* parameters, const AVRational& timeBase);

	void run() override;

	int push(MyAVPacket* pkt, int CurrSerial, bool& exitFlag);

	int clear();

	~AudioThread() override;

	bool isPause{ false };

	void setPause(bool state);

	int setSpeed(const char* _speed);

	int64_t pts{ 0 };
private:
	int init_filter_graph(const char* value);

private:
	int sampleRate{ 0 };
	int format{ 0 };
	uint64_t channel_layout{ 0 };

	AVRational timeBase{};
	AVFilterGraph* filter_graph{ nullptr };
	AVFilterContext* src{ nullptr };
	AVFilterContext* out{ nullptr };
	bool speedFlag{ false };
	const char* speed{ "1.0" };



	int serial{ 0 };
	int currSerial{ 0 };
	std::list<MyAVPacket*> list;
	std::mutex mux;
	std::condition_variable cv;


	Decode* decode{ nullptr };
	Resample* rs{ nullptr };
	AudioPlay* audio{ nullptr };


	bool isExit{ false };
	/*列表最多缓存这么多*/
	static constexpr int maxSize{ 1000 };

	AVFrame* frame{ nullptr };
	uint8_t** pcmData{ nullptr };

	bool display{ false };


};


#endif //PLAYER_AUDIOTHREAD_H
