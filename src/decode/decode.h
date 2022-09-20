#ifndef PLAYER_DECODE_H
#define PLAYER_DECODE_H

#include <mutex>

extern "C" {
#include "libavcodec/avcodec.h"
}

class Decode {
public:

	int open(AVCodecParameters* parameters);

	int clear();

	int send(AVPacket* pkt);

	int receiver(AVFrame* frame);

	~Decode();

private:
	AVCodec* codec{ nullptr };
	AVCodecContext* ctx{ nullptr };

	std::mutex mux;
};


#endif //PLAYER_DECODE_H
