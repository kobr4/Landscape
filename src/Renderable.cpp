#include "Renderable.h"
#include <GL/glew.h>
#include "Texture.h"
#include "Shader.h"
#include "BufferData.h"

#include <stdio.h>
#include <stdlib.h>
#include "OutputConsole.h"

void Renderable::draw() {
	if (glGetError() != GL_NO_ERROR) {
		OutputConsole::log("Renderable::draw() : An error occured before draw.");
		exit(0);
	}

	if (this->texture != NULL) {
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		this->texture->bind();
	}

	
	if (this->secondaryTexture != NULL) {
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		this->secondaryTexture->bind();
	}
	
	if (this->bufferData != NULL) {
		this->bufferData->bind();	
	}

	glVertexPointer (3,GL_FLOAT,nbFloatPerVertex*sizeof(float),0);
	glTexCoordPointer (2,GL_FLOAT,nbFloatPerVertex*sizeof(float),0);
	
	glVertexAttribPointer(Shader::vertexPositionHandle,3,GL_FLOAT,GL_FALSE,nbFloatPerVertex*sizeof(float),0);
	glEnableVertexAttribArray(Shader::vertexPositionHandle);

	glVertexAttribPointer(Shader::texCoordHandle,2,GL_FLOAT,GL_FALSE,nbFloatPerVertex*sizeof(float),(void*)(3*sizeof(float)));
	glEnableVertexAttribArray(Shader::texCoordHandle);

	glDrawArrays (GL_TRIANGLES,0,nbTriangle*3);

	if (this->bufferData != NULL) {
		this->bufferData->unbind();
	}

	if (this->texture) {
		glActiveTexture(GL_TEXTURE0);
		this->texture->unbind();
		glDisable(GL_TEXTURE_2D);
	}	

	
	if (this->secondaryTexture) {
		glActiveTexture(GL_TEXTURE1);
		this->secondaryTexture->unbind();
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}	

	if (glGetError() != GL_NO_ERROR) {
		OutputConsole::log("Renderable::draw() : An error occured.");
	}
}

Renderable * Renderable::createRenderable(Shader * shader, Texture * texture, float * vertexBuffer, unsigned int  triangleCount) {
	Renderable * renderable = new Renderable();
	unsigned int sizeBufferData = triangleCount * 3  * 5 *sizeof(float);
	renderable->setBufferData(new BufferData(vertexBuffer, sizeBufferData));
	renderable->setTexture(texture);
	renderable->setShader(shader);
	renderable->nbTriangle  = triangleCount;
	renderable->model = glm::mat4(1.0f);
	return renderable;
}