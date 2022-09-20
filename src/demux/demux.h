#ifndef PLAYER_DEMUX_H
#define PLAYER_DEMUX_H

#include <mutex>

extern "C" {
#include "libavformat/avformat.h"
};

/*class AVPacket;

class AVCodecParameters;

class AVFormatContext;

class AVStream;

enum AVMediaType;*/

class Demux {
public:

    int open(const char *url);

    int readPacket(AVPacket *pkt) const;

    AVCodecParameters *getVideoParameters() const;

    AVCodecParameters *getAudioParameters() const;

    /* pos是0-1的百分比刻度 */
    int seek(double pos);

    int clear() const;

    int64_t getDuration() const;

    bool isVideo(AVPacket *pkt) const;


    AVRational getAudioTimeBase() const;

    ~Demux();

private:
    int findStreamIdx(AVMediaType type) const;

public:
    /*int width{0};
    int height{0};*/
    /*int channels{0};*/

    AVFormatContext *fmtCtx{nullptr};
private:

    std::mutex mux;

    int audioIdx{0};
    int videoIdx{0};

    AVStream *videoStream{nullptr};
    AVStream *audioStream{nullptr};
};


#endif //PLAYER_DEMUX_H
