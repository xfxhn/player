//
// Created by Administrator on 2022-08-31.
//

#ifndef PLAYER_DEMUXTHREAD_H
#define PLAYER_DEMUXTHREAD_H

#include <mutex>
#include "QThread"
#include "videoCall.h"


struct AVPacket;

class AudioThread;

class VideoThread;

class Demux;


class DemuxThread : public QThread {
public:
    DemuxThread();

    int open(const char *url, VideoCall *call);

    int start();

    void run() override;

    int64_t getCurrentMillisecond();

    int64_t getDuration();

    void setPause(bool state);

    int seek(double pos);


    int setSpeed(const char *speed);

    int clear();

    ~DemuxThread() override;


private:

    int serial{0};
    AVPacket *pkt{nullptr};


    bool isExit{false};
    Demux *demux{nullptr};
    AudioThread *at{nullptr};
    VideoThread *vt{nullptr};

    std::mutex mux;
};


#endif //PLAYER_DEMUXTHREAD_H
