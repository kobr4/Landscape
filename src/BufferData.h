#pragma once
#include <GL/glew.h>

class BufferData {
	unsigned int bufferName;
	void * bufferData;
	int bufferSize;
	GLenum target; 
public :
	BufferData(void * bufferData,int bufferSize, GLenum target = GL_ARRAY_BUFFER);
	void updateBuffer();
	void do_register();
	void bind();
	void unbind();
};