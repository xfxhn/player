#ifndef PLAYER_RESAMPLE_H
#define PLAYER_RESAMPLE_H

extern "C" {
#include "libswresample/swresample.h"
#include "libavcodec/avcodec.h"

}


class Resample {
public:
    int open(AVCodecParameters *parameters);

    int resample(AVFrame *frame, uint8_t **outData);

    void close();

    ~Resample();

private:
    SwrContext *swr_ctx{nullptr};
    AVSampleFormat format{AV_SAMPLE_FMT_S16};
};


#endif //PLAYER_RESAMPLE_H
