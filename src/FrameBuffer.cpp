#include "FrameBuffer.h"
#include <stdlib.h>
#include <GL/glew.h>
#include "Texture.h"
#include "Sprite.h"
#include "Shader.h"
#include <stdio.h>
#include "TextureGenerator.h"


void FrameBuffer::bind(){
	if (this->fb == 0) {
		do_register();
	}

	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);
	//glBindRenderbuffer(GL_RENDERBUFFER, this->depthRb);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRb);	

	// check status
	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("FrameBuffer::bind() ERROR renderTex=%d\n",renderTex);
		exit(0);
	}
}

void FrameBuffer::unbind(int screenWidth, int screenHeight){
	glViewport(0, 0, screenWidth, screenHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::do_register(){
		glGenFramebuffers(1, &this->fb);
		glGenRenderbuffers(1, &this->depthRb);
		glBindRenderbuffer(GL_RENDERBUFFER, this->depthRb);
		if (this->renderTex == 0) {
			glGenTextures(1, &this->renderTex);
			glBindTexture(GL_TEXTURE_2D, this->renderTex);
			pixels = (unsigned char *)calloc(sizeof(char),4 * width * height);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		} else {
			glBindTexture(GL_TEXTURE_2D, this->renderTex);
		}
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);

		glBindRenderbuffer(GL_RENDERBUFFER, this->depthRb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, this->width, this->height);	

		// check status
		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			puts("FrameBuffer::do_register()");
			exit(0);
		}
}

Texture * FrameBuffer::getTexture() {
	return new Texture(this->width,this->height,this->renderTex);
}

void FrameBuffer::draw(unsigned int width,unsigned int height, Shader * shader) {
	if (shader != NULL) {
		shader->bind();
		shader->setProjectionMatrixToOrtho(width,height);
		shader->setModelViewMatrixToIdentity();
		shader->bind_attributes();
	}

	if (this->sprite == NULL) {
		this->sprite = new Sprite(this->getTexture(),(float)(width == 0? this->width : width),(float)(this->height == 0? this->height : height),0,0,1,1);
	}

	this->sprite->draw();

	if (shader != NULL) shader->unbind();
}

void FrameBuffer::blur(unsigned int screenWidth,unsigned int screenHeight) {
	if (this->backFb == NULL) {
		this->backFb = new FrameBuffer(this->width,this->height);
		this->backFb->do_register();
	}
	
	Shader * verticalBlurShader = Shader::createBuiltin(SHADER_BLUR_VERTICAL);
	verticalBlurShader->setProjectionMatrixToOrtho(screenWidth,screenHeight);
	verticalBlurShader->setModelViewMatrixToIdentity();
	
	verticalBlurShader->bind();
	
	this->backFb->bind();
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	this->draw(screenWidth,screenHeight);
	this->backFb->unbind(screenWidth,screenHeight);
	
	verticalBlurShader->unbind();
	

	Shader * horizontalBlurShader = Shader::createBuiltin(SHADER_BLUR_HORIZONTAL);
	
	horizontalBlurShader->setProjectionMatrixToOrtho(screenWidth,screenHeight);
	horizontalBlurShader->setModelViewMatrixToIdentity();
	
	horizontalBlurShader->bind();
	this->bind();
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	this->backFb->draw(screenWidth,screenHeight);
	this->unbind(screenWidth,screenHeight);
	horizontalBlurShader->unbind();
	
}

void FrameBuffer::writeToTGA(char * filename) {
	unsigned char * map = (unsigned char *)malloc(sizeof(unsigned char) * 4 * width * height);
	glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,map);
	TextureGenerator::writeTGA(filename, map, width, height, true);
	free(map);
}