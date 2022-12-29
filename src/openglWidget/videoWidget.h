
#ifndef PLAYER_VIDEOWIDGET_H
#define PLAYER_VIDEOWIDGET_H

#include <fstream>

extern "C" {
#include "libavcodec/avcodec.h"
};
/*创建窗口*/
#include "QOpenGLWidget"
/*调用OpenGL函数*/
#include "QOpenGLFunctions_3_3_Core"
#include "videoCall.h"

class QOpenGLShaderProgram;

class QOpenGLVertexArrayObject;

class QOpenGLBuffer;

class QOpenGLTexture;

class VideoWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core, public VideoCall {


Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);

    int init(int width, int height) override;

    int repaint(AVFrame *frame) override;

    ~VideoWidget() override;

protected:
    void initializeGL() override;

    void paintGL() override;

    void resizeGL(int width, int height) override;

private:
    void initData();

    void initTexture();

private:
    QOpenGLShaderProgram *program = nullptr;
    QOpenGLVertexArrayObject *vao = nullptr;
    QOpenGLBuffer *vbo = nullptr;

    QOpenGLTexture *texture[3]{nullptr};

    int srcWidth{0};
    int srcHeight{0};

    uint8_t *YUVData[3]{nullptr};


};

#endif //PLAYER_VIDEOWIDGET_H
