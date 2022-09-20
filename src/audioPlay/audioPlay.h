
#ifndef PLAYER_AUDIOPLAY_H
#define PLAYER_AUDIOPLAY_H

#include <cstdint>

class AudioPlay {
public:
    static AudioPlay *get();

    virtual int open() = 0;

    virtual int close() = 0;

    virtual int clear() = 0;

    virtual int write(const uint8_t *data, size_t size) = 0;

    virtual int getFree() = 0;

    virtual int64_t getNoPlayPts() = 0;

    virtual void setPause(bool state) = 0;

    ~AudioPlay();


    int sampleRate{44100};
    int sampleSize{16};
    int channels{2};


};


#endif //PLAYER_AUDIOPLAY_H
