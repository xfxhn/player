//
// Created by Administrator on 2022-08-26.
//

#ifndef PLAYER_CAUDIOPLAY_H
#define PLAYER_CAUDIOPLAY_H

#include "audioPlay.h"
#include <mutex>

class QIODevice;

class QAudioOutput;

class CAudioPlay : public AudioPlay {


public:
	QIODevice* io{ nullptr };
	QAudioOutput* output{ nullptr };
public:
	int open() override;

	int close() override;

	int clear() override;

	int write(const uint8_t* data, size_t size) override;


	int getFree() override;


	void setPause(bool state) override;

	int64_t getNoPlayPts() override;
};


#endif //PLAYER_CAUDIOPLAY_H
