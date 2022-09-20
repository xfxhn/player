
#include "videoWidget.h"
#include <iostream>

#include "QOpenGLBuffer"
#include "QOpenGLVertexArrayObject"
#include "QOpenGLShaderProgram"
#include "QOpenGLTexture"
#include "QtCore/QTimer"

VideoWidget::VideoWidget(QWidget* parent)
	: QOpenGLWidget(parent) {

}

int VideoWidget::init(int width, int height) {
	srcWidth = width;
	srcHeight = height;

	delete[]    YUVData[0];
	delete[]    YUVData[1];
	delete[]    YUVData[2];

	YUVData[0] = new uint8_t[srcWidth * srcHeight]();
	YUVData[1] = new uint8_t[srcWidth / 2 * srcHeight / 2]();
	YUVData[2] = new uint8_t[srcWidth / 2 * srcHeight / 2]();

	initTexture();
	return 0;
}

void VideoWidget::initializeGL() {


	bool flag;
	/*init opengl function*/
	initializeOpenGLFunctions();
	/*creator vertex shader*/
	QOpenGLShader* vertShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
	/*compile shader*/
	flag = vertShader->compileSourceFile(":/shader/video.vert");
	if (!flag) {
		fprintf(stderr, "compile vert shader failed");
		return;
	}

	QOpenGLShader* fragShader = new QOpenGLShader(QOpenGLShader::Fragment, this); // NOLINT(modernize-use-auto)
	flag = fragShader->compileSourceFile(":/shader/video.frag");
	if (!flag) {
		fprintf(stderr, "compile frag shader failed");
		return;
	}

	program = new QOpenGLShaderProgram(this);
	program->addShader(vertShader);
	program->addShader(fragShader);
	program->link();


	initData();


	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}


void VideoWidget::paintGL() {


	program->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




	if (YUVData[0]) {
		texture[0]->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, YUVData[0]);
		texture[0]->bind(texture[0]->textureId());
	}

	if (YUVData[1]) {
		texture[1]->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, YUVData[1]);
		texture[1]->bind(texture[1]->textureId());
	}
	if (YUVData[2]) {
		texture[2]->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, YUVData[2]);
		texture[2]->bind(texture[2]->textureId());
	}




	vao->bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

//window change size
void VideoWidget::resizeGL(int width, int height) {
	//QOpenGLWidget::resizeGL(width, height);
}

void VideoWidget::initData() {

	bool flag = false;
	vao = new QOpenGLVertexArrayObject;
	vbo = new QOpenGLBuffer;
	flag = vao->create();
	flag = vbo->create();

	GLfloat vertices[] = {
		// Pos        // Tex
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 1.0f,

		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	vao->bind();
	flag = vbo->bind();


	vbo->allocate(sizeof(vertices));
	vbo->write(0, vertices, sizeof(vertices));

	program->setAttributeBuffer(0, GL_FLOAT, 0, 4, 4 * sizeof(GLfloat));
	program->enableAttributeArray(0);

	/*解除绑定*/
	vbo->release();
	vao->release();
}

void VideoWidget::initTexture() {

	for (int i = 0; i < 3; ++i) {
		texture[i] = new QOpenGLTexture(QOpenGLTexture::Target2D);
		/*create texture object*/
		texture[i]->create();
		/*setting format*/
		texture[i]->setFormat(QOpenGLTexture::R8_UNorm);
		if (i == 0) {
			texture[i]->setSize(VideoWidget::srcWidth, VideoWidget::srcHeight);
		} else {
			texture[i]->setSize(VideoWidget::srcWidth / 2, VideoWidget::srcHeight / 2);
		}
		texture[i]->setMinificationFilter(QOpenGLTexture::Linear);
		texture[i]->setMagnificationFilter(QOpenGLTexture::Linear);
		texture[i]->setWrapMode(QOpenGLTexture::Repeat);

		/*alloc memory*/
		texture[i]->allocateStorage();
	}

	program->bind();
	program->setUniformValue("imageY", texture[0]->textureId());
	program->setUniformValue("imageU", texture[1]->textureId());
	program->setUniformValue("imageV", texture[2]->textureId());
}



int VideoWidget::repaint(AVFrame* frame) {
	if (frame->width != srcWidth || frame->height != srcHeight) {
		//av_frame_free(&frame);
		return -1;
	}

	if (srcWidth == frame->linesize[0]) {
		memcpy(YUVData[0], frame->data[0], srcWidth * srcHeight);
		memcpy(YUVData[1], frame->data[1], srcWidth / 2 * srcHeight / 2);
		memcpy(YUVData[2], frame->data[2], srcWidth / 2 * srcHeight / 2);
	} else {
		for (int i = 0; i < srcHeight; ++i) {
			memcpy(YUVData[0] + srcWidth * i, frame->data[0] + frame->linesize[0] * i, srcWidth);
		}
		for (int i = 0; i < srcHeight / 2; i++) {
			memcpy(YUVData[1] + srcWidth / 2 * i, frame->data[1] + frame->linesize[1] * i, srcWidth / 2);
		}
		for (int i = 0; i < srcHeight / 2; i++) {
			memcpy(YUVData[2] + srcWidth / 2 * i, frame->data[2] + frame->linesize[2] * i, srcWidth / 2);
		}
	}

	update();
	return 0;
}



VideoWidget::~VideoWidget() {
	/*将上下文和当前窗口关联起来*/
	makeCurrent();
	if (vao) {
		vao->destroy();
		delete vao;
	}

	if (vbo) {
		vbo->destroy();
		delete vbo;
	}
	for (int i = 0; i < 3; ++i) {
		if (texture[i]) {
			texture[i]->destroy();
		}

		if (YUVData[i]) {
			int a = 1;
			delete[] YUVData[i];
			YUVData[i] = nullptr;
		}

	}

}
