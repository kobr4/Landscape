#pragma once
#include <stdio.h>
#include <glm/glm.hpp>
class Shader;
class BufferData;
class Texture;
class Renderable {
private :
	Shader * shader;
	BufferData * bufferData;
	BufferData * indexBufferData;
	Texture * texture;
	Texture * secondaryTexture;
	unsigned int nbFloatPerVertex;
	unsigned int nbTriangle;

public :
	Renderable() {
		shader = NULL;
		bufferData = NULL;
		texture = NULL;
		secondaryTexture = NULL;
		nbFloatPerVertex = 5;
		indexBufferData = NULL;
	}
	void setShader(Shader * shader) { this->shader = shader;};
	Shader * getShader() { return shader;};
	void  setBufferData(BufferData * bufferData) { this->bufferData = bufferData;};
	BufferData * getBufferData() { return bufferData;};
	void setTexture(Texture * texture) { this->texture = texture;};
	Texture * getTexture() { return texture;};
	void setSecondaryTexture(Texture * texture) { this->secondaryTexture = texture;};
	Texture * getSecondaryTexture() { return texture;};

	unsigned int getNbFloatPerVertex() { return nbFloatPerVertex; } 
	void draw();
	static Renderable * createRenderable(Shader * shader, Texture * texture, float * vertexBuffer, unsigned int  triangleCount, unsigned int * indexBuffer = NULL, int nbFloatPerVertex = 5);
	glm::mat4 model;
};