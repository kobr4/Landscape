#pragma once;
#include <stdlib.h>
class Texture {
private :
	unsigned int textureName;
	unsigned char * pixels;
public :
	unsigned int width;
	unsigned int height;
	Texture(int width, int height,unsigned char * pixels) {
		this->width = width;
		this->height = height;
		this->pixels = pixels;
		this->textureName = 0;
	}

	Texture(int width, int height,int textureName) {
		this->width = width;
		this->height = height;
		this->textureName = textureName;
	}

	Texture(int width, int height) {
		this->width = width;
		this->height = height;
		this->pixels = (unsigned char *)malloc(sizeof(unsigned char)*width*height*4);
		this->textureName = 0;
	}

	void packTexture(Texture * texture,int top_x,int top_y);
	void blur();
	void merge (Texture * texture);
	void bind();
	void unbind();
	void do_register();
	void update();
	unsigned char *  getPixels();
};