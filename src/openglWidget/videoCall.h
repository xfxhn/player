
#ifndef PLAYER_VIDEOCALL_H
#define PLAYER_VIDEOCALL_H
struct AVFrame;

class VideoCall {
public:
	virtual int init(int width, int height) = 0;

	virtual int repaint(AVFrame* frame) = 0;
};

#endif //PLAYER_VIDEOCALL_H
