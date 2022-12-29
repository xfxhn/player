#include "decode.h"

int Decode::open(AVCodecParameters* parameters) {
	int ret;
	codec = avcodec_find_decoder(parameters->codec_id);
	if (!codec) {
		fprintf(stderr, "Failed to find %s codec\n",
				av_get_media_type_string(parameters->codec_type));
		return AVERROR(EINVAL);
	}

	ctx = avcodec_alloc_context3(codec);
	if (!ctx) {
		fprintf(stderr, "Failed to allocate the %s codec context\n",
				av_get_media_type_string(parameters->codec_type));
		return AVERROR(ENOMEM);
	}

	ret = avcodec_parameters_to_context(ctx, parameters);
	if (ret < 0) {
		fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
				av_get_media_type_string(parameters->codec_type));
		return ret;
	}

	/* 用decode初始化ctx */
	ret = avcodec_open2(ctx, codec, nullptr);
	if (ret < 0) {
		fprintf(stderr, "Failed to open %s codec\n",
				av_get_media_type_string(parameters->codec_type));
		return ret;
	}


	return 0;
}

int Decode::clear() {
	avcodec_flush_buffers(ctx);
	return 0;
}
int Decode::send(AVPacket* pkt) {
	int ret;
	ret = avcodec_send_packet(ctx, pkt);

	if (ret < 0) {
		char error[AV_ERROR_MAX_STRING_SIZE]{ 0 };
		fprintf(stderr, "Error submitting a packet for decoding (%s)\n",
				av_make_error_string(error, AV_ERROR_MAX_STRING_SIZE, ret));
		return ret;
	}
	return 0;
}

int Decode::receiver(AVFrame* frame) {
	int ret = avcodec_receive_frame(ctx, frame);
	return ret;
}

Decode::~Decode() {
	avcodec_free_context(&ctx);
}
