#ifndef BOFANGQI_OPENGLWIDGET_H
#define BOFANGQI_OPENGLWIDGET_H

#include <condition_variable>
#include "QWidget"

QT_BEGIN_NAMESPACE
class DemuxThread;
namespace Ui {
    class mainWindow;
}
QT_END_NAMESPACE

class OpenglWidget : public QWidget {
Q_OBJECT
public:
    explicit OpenglWidget(QWidget *parent = nullptr);

    void timerEvent(QTimerEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

    void setPause() const;

    ~OpenglWidget() override;

public slots:

    int openFile();

    int changePlayState();

    int sliderPress();

    int sliderRelease();


    int handlerChangeSpeed(const char *speed);

private:
    /*是否被按住*/
    bool sliderState{false};
    bool playState{false};


    Ui::mainWindow *ui;

    DemuxThread *de{nullptr};
};

#endif //BOFANGQI_OPENGLWIDGET_H
