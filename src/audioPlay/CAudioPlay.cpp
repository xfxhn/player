#include "CAudioPlay.h"
#include "QAudioFormat"
#include "QAudioOutput"

int CAudioPlay::open() {
	//close();
	QAudioFormat fmt;


	fmt.setSampleRate(sampleRate);
	fmt.setSampleSize(sampleSize);
	fmt.setChannelCount(channels);
	fmt.setCodec("audio/pcm");

	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::UnSignedInt);

	output = new QAudioOutput(fmt);
	io = output->start();
	if (!io) {
		return -1;
	}
	return 0;
}

int CAudioPlay::close() {

	if (io) {
		io->close();
		io = nullptr;
	}
	if (output) {
		output->stop();
		delete output;
		output = nullptr;
	}

	return 0;
}

int CAudioPlay::write(const uint8_t* data, size_t dataSize) {
	if (!data || dataSize <= 0) {
		return -1;
	}
	if (!output || !io) {
		return -1;
	}
	qint64 size = io->write(reinterpret_cast<const char*>(data), dataSize);
	if (size != dataSize) {
		return -1;
	}


	return 0;
}

int CAudioPlay::getFree() {


	if (!output) {
		return 0;
	}
	int free = output->bytesFree();

	return free;
}

int64_t CAudioPlay::getNoPlayPts() {

	/*bufferSize返回音频缓冲区大小*/
	/*bytesFree返回音频缓冲区中可用的空闲字节数*/
	/*计算还未播放的字节大小*/
	double byteSize = output->bufferSize() - output->bytesFree();
	/*一秒字节大小*/
	double secSize = sampleRate * (sampleSize / 8) * channels;

	/*缓冲区还未播放的字节有多少毫秒*/
	return (int64_t)(byteSize / secSize * 1000);
}

void CAudioPlay::setPause(bool state) {

	if (state) {
		/*停止处理音频数据，保留缓冲的音频数据*/
		output->suspend();
	} else {
		/*恢复*/
		output->resume();
	}

}

int CAudioPlay::clear() {
	/*删除缓冲区中的所有音频数据，将缓冲区重置为零。*/
	/*int b = output->bufferSize();
	output->reset();
	int a = output->bufferSize();
	output->setBufferSize(b);
	a = output->bufferSize();*/
	return 0;
}
