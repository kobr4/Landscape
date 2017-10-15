#pragma once

class BufferData {
	unsigned int bufferName;
	void * bufferData;
	int bufferSize;
public :
	BufferData(void * bufferData,int bufferSize);
	void updateBuffer();
	void do_register();
	void bind();
	void unbind();
};