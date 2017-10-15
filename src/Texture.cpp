#include "Texture.h"
#include <GL/glew.h>

void Texture::bind() {
	if (this->textureName == 0) {
		do_register();
	}
	glBindTexture(GL_TEXTURE_2D, this->textureName); 
}

void Texture::unbind() {
	glBindTexture(GL_TEXTURE_2D, 0); 
}

void Texture::update() {
	if (this->textureName != 0) {
		glBindTexture(GL_TEXTURE_2D, this->textureName); 
				
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);
	}
}

void Texture::do_register() {
	glGenTextures(1, &this->textureName); 
	this->update();
}

void Texture::blur() { 
	for (int i = 0;i < this->width;i++) { 
		for (int j = 0;j < this->height;j++) {
			unsigned int offset = i + j *this->width;
			int offsetUp = (offset-this->width)*4;
			int offsetRight = (offset+this->width)*4;
			if (j == 0) offsetUp = (i + (this->height - 1) * this->width - this->width) * 4;
			if (j == this->height -1) offsetRight = (i + this->width)*4;
			this->pixels[offset*4] = (this->pixels[offsetUp] + this->pixels[offsetRight])/2;
			this->pixels[offset*4+1] = (this->pixels[offsetUp+1] + this->pixels[offsetRight+1])/2;
			this->pixels[offset*4+2] = (this->pixels[offsetUp+2] + this->pixels[offsetRight+2])/2;
			this->pixels[offset*4+3] = (this->pixels[offsetUp+3] + this->pixels[offsetRight+3])/2;
		}
	}

	for (int i = 0;i < this->width;i++) { 
		for (int j = 0;j < this->height;j++) {
			unsigned int offset = i+ j*this->width;
			int offsetLeft = (offset-1)*4;
			int offsetRight = (offset+1)*4;
			if (offset == 0)  offsetLeft = (this->width - 1) + (this->height - 1) * this->width;
			if (offset ==  (this->width - 1) + (this->height - 1) * this->width) offsetRight = 0;
			this->pixels[offset*4] = (this->pixels[offsetLeft] + this->pixels[offsetRight])/2;
			this->pixels[offset*4+1] = (this->pixels[offsetLeft+1] + this->pixels[offsetRight+1])/2;
			this->pixels[offset*4+2] = (this->pixels[offsetLeft+2] + this->pixels[offsetRight+2])/2;
			this->pixels[offset*4+3] = (this->pixels[offsetLeft+3] + this->pixels[offsetRight+3])/2;
		}
	}
}

void Texture::merge(Texture * texture) {
	unsigned int offset = 0;
	for (int i = 0;i < texture->width;i++) { 
		for (int j = 0;j < texture->height;j++) {
			unsigned int a = texture->pixels[offset] + texture->pixels[offset+1] + texture->pixels[offset+2];
			unsigned int b = this->pixels[offset] + this->pixels[offset+1] + this->pixels[offset+2];
			if (a > b) {
				this->pixels[offset] = texture->pixels[offset];
				this->pixels[offset+1] = texture->pixels[offset+1];
				this->pixels[offset+2] = texture->pixels[offset+2];
				this->pixels[offset+3] = texture->pixels[offset+3];
			}
			offset = offset + 4;
		}
	}
}

void Texture::packTexture(Texture * texture,int top_x,int top_y) {
	for (int i = 0;i < texture->width;i++) { 
		for (int j = 0;j < texture->height;j++) {
			int x = i + top_x;
			int y = j + top_y;
			this->pixels[(x+ y*this->width)*4] = texture->pixels[(i+j*texture->width)*4];
			this->pixels[(x+ y*this->width)*4+1] = texture->pixels[(i+j*texture->width)*4+1];
			this->pixels[(x+ y*this->width)*4+2] = texture->pixels[(i+j*texture->width)*4+2];
			this->pixels[(x+ y*this->width)*4+3] = texture->pixels[(i+j*texture->width)*4+3];
		}
	}
}

unsigned char * Texture::getPixels() {
	return this->pixels;
}