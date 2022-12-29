#include "demux.h"
#include <iostream>


int Demux::open(const char *url) {

    int ret;

    AVDictionary *opts = nullptr;

    /* 设置传输方式，默认是udp，可能会出现帧乱序现象 */
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    /* 设置最大延时 */
    av_dict_set(&opts, "max_delay", "500", 0);

    /* 没有分配fmtCtx的话，会在avformat_open_input去分配 */
    ret = avformat_open_input(&fmtCtx, url, nullptr, &opts);
    if (ret < 0) {
        fprintf(stderr, "Could not open source file %s\n", url);
        return ret;
    }

    ret = avformat_find_stream_info(fmtCtx, nullptr);
    if (ret < 0) {
        fprintf(stderr, "Could not find stream information\n");
        return -1;
    }


    videoIdx = findStreamIdx(AVMEDIA_TYPE_VIDEO);
    audioIdx = findStreamIdx(AVMEDIA_TYPE_AUDIO);

    videoStream = fmtCtx->streams[videoIdx];

    /*width = videoStream->codecpar->width;
    height = videoStream->codecpar->height;*/


    audioStream = fmtCtx->streams[audioIdx];

    //channels = audioStream->codecpar->channels;
    return ret;
}


/*返回视频时长毫秒*/
int64_t Demux::getDuration() const {
    if (fmtCtx) {
        return fmtCtx->duration / 1000;
    }
    return 0;
}

int Demux::readPacket(AVPacket *pkt) const {
    int ret;

    ret = av_read_frame(fmtCtx, pkt);

    if (ret < 0) {
        if (ret == AVERROR_EOF || avio_feof(fmtCtx->pb)) {
            /*读取完毕*/
            return 1;
        }
        if (fmtCtx->pb && fmtCtx->pb->error) {
            /*发生错误*/
            return -1;
        }
        /*都不是，等一下，根据ffplay代码里*/
        return 2;
    }
    /*转为毫秒*/
    pkt->pts = pkt->pts * (1000 * (av_q2d(fmtCtx->streams[pkt->stream_index]->time_base)));
    pkt->dts = pkt->dts * (1000 * (av_q2d(fmtCtx->streams[pkt->stream_index]->time_base)));
    return 0;
}

int Demux::findStreamIdx(AVMediaType type) const {
    return av_find_best_stream(fmtCtx, type, -1, -1, nullptr, 0);
}


AVCodecParameters *Demux::getVideoParameters() const {
    AVCodecParameters *parameters = avcodec_parameters_alloc();
    avcodec_parameters_copy(parameters, fmtCtx->streams[videoIdx]->codecpar);
    return parameters;
}

AVCodecParameters *Demux::getAudioParameters() const {
    AVCodecParameters *parameters = avcodec_parameters_alloc();
    avcodec_parameters_copy(parameters, fmtCtx->streams[audioIdx]->codecpar);
    return parameters;
}

int Demux::seek(double pos) {
    //avformat_flush(fmtCtx);

    const double timeBase = av_q2d(videoStream->time_base);
    /* 获取总时长 */
    double duration = videoStream->duration * timeBase;

    /*获取要跳转到几秒*/
    int sec = pos * duration;
    //int64_t seekPos = pos * videoStream->duration;

    /*
     * 参数timestamp就表示AVPacket中的pts时间戳
     * 因为packet播放时刻值：sec(单位秒) = packet.pts * av_q2d(stream.time_base);
     * 所以pts = sec / av_q2d(fmtCtx->streams[vStreamIndex]->time_base)
     * */
    int64_t pts = sec / timeBase;
    /* AVSEEK_FLAG_BACKWARD往后找，AVSEEK_FLAG_FRAME找到关键帧 */

    int ret = av_seek_frame(fmtCtx, videoIdx, pts, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);

    if (ret < 0) {
        return ret;
    }
    return 0;
}

int Demux::clear() const {
    /*刷新所有缓冲数据*/
    avformat_flush(fmtCtx);
    return 0;
}


bool Demux::isVideo(AVPacket *packet) const {
    if (!packet) {
        return false;
    }
    return packet->stream_index == videoIdx;
}

Demux::~Demux() {
    avformat_close_input(&fmtCtx);
}

AVRational Demux::getAudioTimeBase() const {
    return audioStream->time_base;
}


