#include "BufferData.h"
#include <GL/glew.h>

BufferData::BufferData(void * bufferData,int bufferSize, GLenum target) {
	this->bufferData = bufferData;
	this->bufferSize = bufferSize;
	this->bufferName = 0;
	this->target = target;
}

void BufferData::do_register() {
	glGenBuffers(1,&this->bufferName);
	updateBuffer();
}

void BufferData::updateBuffer() {
	glBindBuffer(target, this->bufferName);
	glBufferData(target,this->bufferSize,this->bufferData,GL_STATIC_DRAW);   
}

void BufferData::bind() {
	if (this->bufferName == 0) {
		do_register();
	}
	glBindBuffer(target, this->bufferName);
}
	
void BufferData::unbind() {
	glBindBuffer(target, 0);
}