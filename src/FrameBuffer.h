#pragma once
#include <stdio.h>

class Texture;
class Sprite;
class Shader;

class FrameBuffer {
private :
	unsigned int fb;
	unsigned int depthRb;
	unsigned int renderTex;
	int width;
	int height;
	unsigned char * pixels;
	Sprite * sprite;
	FrameBuffer * backFb;
public :
	FrameBuffer(int width,int height, int textureId = 0) {
		this->width = width;
		this->height = height;
		this->fb = 0;
		this->depthRb = 0;
		this->renderTex = textureId;
		this->sprite = NULL;
		this->backFb = NULL;
	}
	void bind();
	void unbind(int screenWidth, int screenHeight);
	void do_register();
	int getTextureId() {
		return renderTex;
	}
	Texture * getTexture();
	void draw(unsigned int width = 0,unsigned int height = 0,Shader * shader = NULL);
	void blur(unsigned int screenWidth,unsigned int screenHeight);
	void writeToTGA(char * filename);
};
