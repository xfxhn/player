#include "resample.h"

extern "C" {
#include "libavutil/opt.h"
}

int Resample::open(AVCodecParameters *parameters) {



    int ret;
    /* 创建重采样上下文 分配音频重采样的上下文 */
    swr_ctx = swr_alloc();
    if (!swr_ctx) {
        fprintf(stderr, "Could not allocate resampler context\n");
        return AVERROR(ENOMEM);
    }

    //设置输入通道数
    av_opt_set_int(swr_ctx, "in_channel_layout", parameters->channel_layout, 0);
    //设置输入采样率
    av_opt_set_int(swr_ctx, "in_sample_rate", parameters->sample_rate, 0);
    //设置输入格式
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", AVSampleFormat(parameters->format), 0);


    //设置输出通道数
    av_opt_set_int(swr_ctx, "out_channel_layout", parameters->channel_layout, 0);
    //设置输出采样率
    av_opt_set_int(swr_ctx, "out_sample_rate", parameters->sample_rate, 0);
    //设置输出格式
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", format, 0);


    ret = swr_init(swr_ctx);
    if (ret < 0) {
        fprintf(stderr, "Failed to initialize the resampling context\n");
        return ret;
    }
    return 0;
}

int Resample::resample(AVFrame *frame, uint8_t **outData) {
    if (!frame) {
        fprintf(stderr, "frame is empty!");
        return -1;
    }
    int ret = swr_convert(swr_ctx, outData, frame->nb_samples,
                          (const uint8_t **) frame->data, frame->nb_samples);
    if (ret < 0) {
        fprintf(stderr, "Error while converting\n");
        return ret;
    }
    return av_samples_get_buffer_size(nullptr, frame->channels,
                                      ret, format, 1);
}

void Resample::close() {
    swr_free(&swr_ctx);
}

Resample::~Resample() {
    if (swr_ctx) {
        close();
    }
}
