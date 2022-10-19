#include "QFiledialog"
#include "openglWidget.h"
#include "ui_videoWidget.h"
#include "demuxThread.h"

OpenglWidget::OpenglWidget(QWidget* parent)
	: QWidget(parent), ui(new Ui::mainWindow) {

	/*[&]() {
		setPause();
	};*/
	ui->setupUi(this);


	QObject::connect(ui->pushButton_2, &QPushButton::clicked, this, [&]() {
		handlerChangeSpeed("1.0");
					 });
	QObject::connect(ui->pushButton_3, &QPushButton::clicked, this, [&]() {
		handlerChangeSpeed("1.5");
					 });
	QObject::connect(ui->pushButton_4, &QPushButton::clicked, this, [&]() {
		handlerChangeSpeed("2.0");
					 });

}

void OpenglWidget::timerEvent(QTimerEvent* event) {
	if (sliderState) return;
	int64_t duration = de->getDuration();
	int64_t currentTime = de->getCurrentMillisecond();

	if (duration > 0) {
		double pos = (double)currentTime / (double)duration;
		ui->progressBar->setValue(ui->progressBar->maximum() * pos);
	}
}

int OpenglWidget::openFile() {

	int ret;
	QString name = QFileDialog::getOpenFileName(this, QString("选择视频文件"));

	if (name.isEmpty()) {
		return -1;
	}

	this->setWindowTitle(name);


	delete de;
	de = new DemuxThread;
	//name.toLatin1().data()

	ret = de->open(
		name.toLatin1().data(),
		ui->openGLWidget);
	if (ret < 0) {
		fprintf(stderr, "open %s fialed\n", name.toLatin1().data());
		exit(-1);
	}
	ui->playbtn->setText("暂停");
	startTimer(10);


	/* 先启动线程，不要每次重新打开一个文件，就又重新开启线程 */
	de->start();

	return 0;


}


void OpenglWidget::resizeEvent(QResizeEvent* event) {
	//QWidget::resizeEvent(event);
	/*设置部件大小*/
	ui->openGLWidget->resize(this->size());
}

void OpenglWidget::setPause() const {
	if (playState) {
		ui->playbtn->setText("播放");
	} else {
		ui->playbtn->setText("暂停");
	}
}

int OpenglWidget::changePlayState() {

	if (!de) {
		return 0;
	}

	playState = !playState;
	if (playState) {
		ui->playbtn->setText("播放");
	} else {
		ui->playbtn->setText("暂停");
	}

	de->setPause(playState);
	return 0;
}

int OpenglWidget::sliderPress() {

	sliderState = true;

	return 0;
}

int OpenglWidget::sliderRelease() {
	sliderState = false;
	double pos = (double)ui->progressBar->value() / (double)ui->progressBar->maximum();

	if (de) {
		de->seek(pos);
	}

	return 0;
}


OpenglWidget::~OpenglWidget() {
	delete de;
	delete ui;
}

int OpenglWidget::handlerChangeSpeed(const char* speed) {

	de->setSpeed(speed);
	return 0;
}
