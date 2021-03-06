/*
* Copyright (c) 2012, Nicolas My
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the <organization> nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Nicolas My ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Nicolas My BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//#define GLEW_STATIC
#include "Renderer.h"
#define GLEW_STATIC
#include <SDL.h>
#undef main
#include <SDL_ttf.h>
#include <iostream>

#include <GL/glew.h>
#include "Shader.h"
#include "FrameBuffer.h"
#include <glm/glm.hpp>
#include <glm\gtx\rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm\gtx\intersect.hpp>
#include "Sprite.h"
#include "Texture.h"
#include "FastMath.h"
#include "Camera.h"
#include "OutputConsole.h"
#include "TextureGenerator.h"
#include <time.h>       /* time */
#include "UIWidget.h"

#include <windows.h>

//ugliest hack ever !
const char test[] = {
#include "font.data"
};

const char * fragment_lightmap_texturing =
#include "fragment_lightmap_texturing.gl"
;

const char * vertex = 
#include "vertex.gl"
;

const char * vertex_terrain = 
#include "vertex_terrain.gl"
;
	
const char * fragment_debug =
#include "fragment_debug.gl"
;

const char * fragment_skybox =
#include "fragment_skybox.gl"
;

const char * fragment_terrain =
#include "fragment_terrain.gl"
;

unsigned char g_texdata[]= { 255, 255, 255, 255, 255, 255, 255, 255,
							255, 255, 255, 255, 255, 255, 255, 255};


unsigned char  g_checker_texdata[]= { 255, 255, 255, 255, 0, 0, 0, 0,
							0, 0, 0, 0, 255, 255, 255, 255};

int thread_func(void *data);
SDL_Surface* CreateSurface(Uint32 flags,int width,int height,const SDL_Surface* display);



#include "Renderable.h"
//Renderable * g_renderable;
std::vector<Renderable*> g_renderableList = std::vector<Renderable*>();
Renderable * sphereRenderable;

Shader * g_shader_skybox;
Shader * g_shader_terrain;

float * g_vertexBuffer = NULL;
Texture * g_textureHeight = TextureGenerator::generatePerlinNoiseGreyscale(4096,4096);
Renderable  * g_SkySphere = NULL;

SDL_Joystick * g_joystick = NULL; // on cr�e le joystick
bool g_collision_detection = true;
bool g_cullface = true;
bool g_postprocess = false;
volatile bool g_asyncload = false;
float g_radius = 6378.f;

typedef struct T_QUADNODE {
	struct T_QUADNODE * left;
	struct T_QUADNODE * right;
	struct T_QUADNODE * up;
	struct T_QUADNODE * down;
	struct T_QUADNODE * sublodList;
	unsigned int sublodSize;
	Renderable * renderable;
	float * vertexBuffer;
	unsigned int * indexBuffer;
	int triangleCount;
	glm::vec3 normal;
	glm::vec3 position;
	glm::vec3 a;
	float aHeight;
	glm::vec3 b;
	float bHeight;
	glm::vec3 c;
	float cHeight;
	glm::vec3 d;
	float dHeight;
	float drawDistance;
	float heightFactor;
} T_QUADNODE;

T_QUADNODE * g_quadnodeList = NULL;

void func_joystick_cb(void * data) {
	if (g_joystick == NULL) {
		g_joystick = SDL_JoystickOpen(0);
	}
	OutputConsole::log("Force joystick detection");
}

void func_bool_collision_change_cb(bool newState, void * data) {
	g_collision_detection = newState;
}

void func_bool_cullface_change_cb(bool newState, void * data) {
	g_cullface = newState;
}

void func_bool_postprocess_change_cb(bool newState, void * data) {
	g_postprocess = newState;
}

void func_exit_cb(void * data) {
	Renderer * renderer = (Renderer*)data;
	renderer->setExitState();

}

void func_resume_cb(void * data) {
	UIWidget::currentWidget->setActive(false);
	SDL_SetRelativeMouseMode(SDL_TRUE);
}

void func_back_cb(void * data) {
	UIWidget * widget = (UIWidget *)data;
	if (widget->getParent() != NULL && widget->getParent()->getParent() != NULL) {
		UIWidget::currentWidget = widget->getParent()->getParent();
	}

}

void copyvector(float * vertexBuffer, int offset, glm::vec3 pt, glm::vec2 ptex) {
	vertexBuffer[offset] = pt[0];
	vertexBuffer[offset+1] = pt[1];
	vertexBuffer[offset+2] = pt[2];
	vertexBuffer[offset+3] = ptex[0];
	vertexBuffer[offset+4] = ptex[1];
}

void addVertex(float * vertexBuffer, int vertexIndex, float x, float y, float z, float u, float v) {
	vertexBuffer[vertexIndex*5] = x;
	vertexBuffer[vertexIndex*5+1] = y;
	vertexBuffer[vertexIndex*5+2] = z;
	vertexBuffer[vertexIndex*5+3] = u;
	vertexBuffer[vertexIndex*5+4] = v;
}

void addVertexNormal(float * vertexBuffer, int vertexIndex, float x, float y, float z, float u, float v, float nx, float ny, float nz) {
	unsigned int offset = vertexIndex<<3;
	vertexBuffer[offset+0] = x;
	vertexBuffer[offset+1] = y;
	vertexBuffer[offset+2] = z;
	vertexBuffer[offset+3] = u;
	vertexBuffer[offset+4] = v;
	vertexBuffer[offset+5] = nx;
	vertexBuffer[offset+6] = ny;
	vertexBuffer[offset+7] = nz;
}


float * generateHeighmap(int size, float unit, Texture * textureHeight) {
	float * vertexBuffer = (float*)malloc(sizeof(float) * size * size * 2 * 5 * 3);
	unsigned char * heightmap = textureHeight->getPixels();
	int vertexIndex = 0;
	for (int i = 0;i < size;i++) {
		for(int j = 0;j < size;j++) {
			float fSize = (float)size;
			float v[] = { (float)i/fSize, (float)j/fSize, (float)(i+1)/fSize,  (float)(j+1)/fSize};
			addVertex(vertexBuffer, vertexIndex++, i*unit, heightmap[(i + j * size)*4], j*unit, v[0], v[1]);
			addVertex(vertexBuffer, vertexIndex++, (i+1)*unit, heightmap[(((i+1)&0xff) + ((j+1)&0xff) * size)*4],(j+1)*unit, v[2], v[3]);
			addVertex(vertexBuffer, vertexIndex++, (i+1)*unit, heightmap[(((i+1)&0xff) + j * size)*4], j*unit, v[2], v[1]);
			addVertex(vertexBuffer, vertexIndex++,i*unit,heightmap[(i + j * size)*4], j*unit,v[0],v[1]);
			addVertex(vertexBuffer, vertexIndex++,(i)*unit, heightmap[(((i)&0xff) + ((j+1)&0xff) * size)*4],(j+1)*unit,v[0],v[3]);
			addVertex(vertexBuffer, vertexIndex++,(i+1)*unit,heightmap[(((i+1)&0xff) + ((j+1)&0xff) * size)*4],(j+1)*unit, v[2],v[3]);
		}
	}
	return vertexBuffer;
}


glm::vec3 bilinearInterpolation(float x, float y, float x1, float x2, float y1, float y2, glm::vec3 fx1y1,glm::vec3 fx2y1,glm::vec3 fx2y2,glm::vec3 fx1y2) {
	glm::vec3 deltafxy = fx1y1 + fx2y2 - fx2y1 - fx1y2;
	glm::vec3 deltafx = fx2y1 - fx1y1;
	glm::vec3 deltafy = fx1y2 - fx1y1;
	float dx = x - x1;
	float dy = y - y1;
	float deltax = x2 - x1;
	float deltay = y2 - y1;
	return deltafx * (dx/deltax) + deltafy * (dy/deltay) + deltafxy * (dx / deltax) * (dy /deltay) + fx1y1;

}

float bilinearInterpolation(float x, float y, float x1, float x2, float y1, float y2, float fx1y1, float fx2y1, float fx2y2, float fx1y2) {
	float deltafxy = fx1y1 + fx2y2 - fx2y1 - fx1y2;
	float deltafx = fx2y1 - fx1y1;
	float deltafy = fx1y2 - fx1y1;
	float dx = x - x1;
	float dy = y - y1;
	float deltax = x2 - x1;
	float deltay = y2 - y1;
	return deltafx * (dx/deltax) + deltafy * (dy/deltay) + deltafxy * (dx / deltax) * (dy /deltay) + fx1y1;
}



float getHeightmapInterpolation(Texture * textureHeight, float ut1, float vt1) {
	unsigned char * heightmap = textureHeight->getPixels();
	int iU1 = ((int)(ut1 * (float)textureHeight->width)) & (textureHeight->width-1); 
	int iV1 = ((int)(vt1 * (float)textureHeight->height)) & (textureHeight->height -1);
	float heightFactor1 = ((float)heightmap[(iU1 + iV1 * textureHeight->width) << 2] - 127.f) / 127.f;
	return heightFactor1;
}


glm::vec3 computeNormal(glm::vec3 const & a, glm::vec3 const & b, glm::vec3 const & c) {
        return glm::normalize(glm::cross(c - a, b - a));
}

float * generateHeighmap(int size, Texture * textureHeight, 
                         glm::vec3 a, float aHeight, glm::vec3 b , float bHeight, glm::vec3 c, float cHeight, glm::vec3 d, float dHeight,
                         float u1, float v1, float u2, float v2, float heightFactor, unsigned int ** indexBuffer, bool bPreserveRadius, float radius) {
    float * vertexBuffer = (float*)malloc(sizeof(float) * size * size * 2 * 8 * 3);
	*indexBuffer =  (unsigned int*)malloc(sizeof(unsigned int) *  size * size * 2 * 3);
	unsigned int  * indexBufferPtr = *indexBuffer;

    unsigned char * heightmap = textureHeight->getPixels();
    int vertexIndex = 0;
	int faceIndex = 0;
    float fSize = (float)size;
	float ut = (u2 - u1) / fSize;
	float vt = (v2 - v1) / fSize;	
    for (int i = 0;i < size;i++) {
        for(int j = 0;j < size;j++) {          
			float v[] = { (float)i/fSize, (float)j/fSize, (float)(i+1)/fSize,  (float)(j+1)/fSize};
		
			float ut1 = u1 + ut * (float)i; 
			float vt1 = v1 + vt * (float)j;
			float ut2 = u1 + ut * (float)(i+1); 
			float vt2 = v1 + vt * (float)(j+1);
			
			float p1height = bilinearInterpolation(v[0],v[1],0.f,1.f,0.f,1.f,aHeight,bHeight,cHeight,dHeight);
			float p2height = bilinearInterpolation(v[2],v[1],0.f,1.f,0.f,1.f,aHeight,bHeight,cHeight,dHeight);
			float p3height = bilinearInterpolation(v[2],v[3],0.f,1.f,0.f,1.f,aHeight,bHeight,cHeight,dHeight);
			float p4height = bilinearInterpolation(v[0],v[3],0.f,1.f,0.f,1.f,aHeight,bHeight,cHeight,dHeight);

			float heightFactor1 = getHeightmapInterpolation(textureHeight,ut1,vt1) * heightFactor;
			float heightFactor2 = getHeightmapInterpolation(textureHeight,ut2,vt1) * heightFactor;
			float heightFactor3 = getHeightmapInterpolation(textureHeight,ut2,vt2) * heightFactor;
			float heightFactor4 = getHeightmapInterpolation(textureHeight,ut1,vt2) * heightFactor;
			
			glm::vec3 p1sphere = bilinearInterpolation(i,j,0,size,0,size,a,b,c,d);
			glm::vec3 p2sphere = bilinearInterpolation(i+1,j,0,size,0,size,a,b,c,d);
			glm::vec3 p3sphere = bilinearInterpolation(i+1,j+1,0,size,0,size,a,b,c,d);
			glm::vec3 p4sphere = bilinearInterpolation(i,j+1,0,size,0,size,a,b,c,d);
      
			if (bPreserveRadius) {
				p1sphere = radius / glm::length(p1sphere) *  p1sphere;
				p2sphere = radius / glm::length(p2sphere) *  p2sphere;
				p3sphere = radius / glm::length(p3sphere) *  p3sphere;
				p4sphere = radius / glm::length(p4sphere) *  p4sphere;
			}

			glm::vec3 p1 = p1sphere + glm::normalize(p1sphere) * (heightFactor1 + p1height);
			glm::vec3 p2 = p2sphere + glm::normalize(p2sphere) * (heightFactor2 + p2height);
			glm::vec3 p3 = p3sphere + glm::normalize(p3sphere) * (heightFactor3 + p3height);
			glm::vec3 p4 = p4sphere + glm::normalize(p4sphere) * (heightFactor4 + p4height);

			
			glm::vec3 n1 = computeNormal(p3, p2, p1);
			glm::vec3 n2 = computeNormal(p4, p3, p1);
			glm::vec3 np1 = glm::normalize((n1 + n2)/2.f); 

			indexBufferPtr[faceIndex++] = vertexIndex+2;
			indexBufferPtr[faceIndex++] = vertexIndex+1;
			indexBufferPtr[faceIndex++] = vertexIndex+0;
			indexBufferPtr[faceIndex++] = vertexIndex+3;
			indexBufferPtr[faceIndex++] = vertexIndex+2;
			indexBufferPtr[faceIndex++] = vertexIndex+0;

            addVertexNormal(vertexBuffer, vertexIndex++, p1.x, p1.y, p1.z, ut1, vt1, np1.x, np1.y, np1.z);
            addVertexNormal(vertexBuffer, vertexIndex++, p2.x, p2.y, p2.z, ut2, vt1, n1.x, n1.y, n1.z);
            addVertexNormal(vertexBuffer, vertexIndex++, p3.x, p3.y, p3.z, ut2, vt2, np1.x, np1.y, np1.z);
			addVertexNormal(vertexBuffer, vertexIndex++, p4.x, p4.y, p4.z, ut1, vt2, n2.x, n2.y, n2.z);
        }
    }
    return vertexBuffer;
}


float *  generateSphere( float radius, unsigned int nbSlices) {
	int triangleCount = nbSlices * nbSlices * 2;
	float * vertexBuffer = (float*)malloc(sizeof(float) * triangleCount  * 3 * 5);
	unsigned int count = 0;
	for (int i = 0;i < nbSlices;i++) {
		for (int j = 0;j < nbSlices;j++) {

			float alpha = glm::pi<float>()*1 / (nbSlices+1) * i;
            float beta = glm::pi<float>()*2 / nbSlices * j;
            float alpha1 = glm::pi<float>()*1 / (nbSlices+1) * (i + 1);
            float beta1 = glm::pi<float>()*2 / nbSlices * (j + 1);


			glm::vec3 a = glm::vec3(radius * sin(alpha) * cos(beta), radius * cos(alpha),radius * sin(alpha) * sin(beta));
            glm::vec3 b = glm::vec3(radius * sin(alpha) * cos(beta1), radius * cos(alpha),radius * sin(alpha) * sin(beta1));
            glm::vec3 c = glm::vec3(radius * sin(alpha1) * cos(beta1), radius * cos(alpha1),radius * sin(alpha1) * sin(beta1));
            glm::vec3 d = glm::vec3(radius * sin(alpha1) * cos(beta), radius * cos(alpha1),radius * sin(alpha1) * sin(beta));

			for (int k = 0;k < 3;k++) vertexBuffer[count++] = a[k];
			vertexBuffer[count++] = (float)i/(float)nbSlices;
			vertexBuffer[count++] = (float)j/(float)nbSlices;
			for (int k = 0;k < 3;k++) vertexBuffer[count++] = c[k];
			vertexBuffer[count++] = (float)(i+1)/(float)nbSlices;
			vertexBuffer[count++] = (float)(j+1)/nbSlices;
			for (int k = 0;k < 3;k++) vertexBuffer[count++] = b[k];
			vertexBuffer[count++] = (float)(i+1)/nbSlices;
			vertexBuffer[count++] = (float)(j)/nbSlices;
			for (int k = 0;k < 3;k++) vertexBuffer[count++] = a[k];
			vertexBuffer[count++] = (float)i/nbSlices;
			vertexBuffer[count++] = (float)j/nbSlices;
			for (int k = 0;k < 3;k++) vertexBuffer[count++] = d[k];
			vertexBuffer[count++] = (float)(i)/nbSlices;
			vertexBuffer[count++] = (float)(j+1)/nbSlices;	
			for (int k = 0;k < 3;k++) vertexBuffer[count++] = c[k];
			vertexBuffer[count++] = (float)(i+1)/nbSlices;
			vertexBuffer[count++] = (float)(j+1)/nbSlices;
		}
	}

	return vertexBuffer;
}

T_QUADNODE * generateQuadMesh(unsigned int size, Texture * textureHeight, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
							  float aHeight, float bHeight, float cHeight, float dHeight,
							  float drawDistance, float heightFactor, float radius) {
	int quadCount = size * size;
	T_QUADNODE * quadBuffer = (T_QUADNODE *)malloc(sizeof(T_QUADNODE) * quadCount);

	for (int i = 0;i < quadCount;i++) {
		quadBuffer[i].down = NULL;
		quadBuffer[i].left = NULL;
		quadBuffer[i].renderable = NULL;
		quadBuffer[i].right = NULL;
		quadBuffer[i].up = NULL;
		quadBuffer[i].vertexBuffer = NULL;
		quadBuffer[i].sublodList = NULL;
	}
	float ut = (1.f - 0.f) / (float)size;
	float vt = (1.f - 0.f) / (float)size;
	unsigned int count = 0;
	for (int i = 0;i < size;i++) {
		for (int j = 0;j < size;j++) {
			int hmapSize = 16;
			float v[] = { (float)i/size, (float)j/size, (float)(i+1)/size,  (float)(j+1)/size};

			float ut1 = 0.f + ut * (float)i; 
			float vt1 = 0.f + vt * (float)j;
			float ut2 = 0.f + ut * (float)(i+1); 
			float vt2 = 0.f + vt * (float)(j+1);

			glm::vec3 p1sphere = bilinearInterpolation(i,j,0,size,0,size,a,b,c,d);
			glm::vec3 p2sphere = bilinearInterpolation(i+1,j,0,size,0,size,a,b,c,d);
			glm::vec3 p3sphere = bilinearInterpolation(i+1,j+1,0,size,0,size,a,b,c,d);
			glm::vec3 p4sphere = bilinearInterpolation(i,j+1,0,size,0,size,a,b,c,d);


			float heightFactor1 = getHeightmapInterpolation(textureHeight,ut1,vt1) * heightFactor;
			float heightFactor2 = getHeightmapInterpolation(textureHeight,ut2,vt1) * heightFactor;
			float heightFactor3 = getHeightmapInterpolation(textureHeight,ut2,vt2) * heightFactor;
			float heightFactor4 = getHeightmapInterpolation(textureHeight,ut1,vt2) * heightFactor;

			float p1Height = bilinearInterpolation(v[0],v[1],0.f,1.f,0.f,1.f,aHeight,bHeight,cHeight,dHeight);
			float p2Height = bilinearInterpolation(v[2],v[1],0.f,1.f,0.f,1.f,aHeight,bHeight,cHeight,dHeight);
			float p3Height = bilinearInterpolation(v[2],v[3],0.f,1.f,0.f,1.f,aHeight,bHeight,cHeight,dHeight);
			float p4Height = bilinearInterpolation(v[0],v[3],0.f,1.f,0.f,1.f,aHeight,bHeight,cHeight,dHeight);

			unsigned int *indexBuffer;
			float * vertexBuffer = generateHeighmap(hmapSize, textureHeight, p1sphere, p1Height, p2sphere, p2Height, p3sphere, p3Height, p4sphere, p4Height, ut1, vt1, ut2, vt2,heightFactor, &indexBuffer, true, radius);

			quadBuffer[i + j*size].vertexBuffer = vertexBuffer; 
			quadBuffer[i + j*size].indexBuffer = indexBuffer;
			quadBuffer[i + j*size].triangleCount = hmapSize * hmapSize * 2;
			quadBuffer[i + j*size].position = (p1sphere + p2sphere + p3sphere + p4sphere) * 0.25f; 
			quadBuffer[i + j*size].normal = glm::normalize(quadBuffer[i + j*size].position);
			quadBuffer[i + j*size].a = p1sphere;
			quadBuffer[i + j*size].aHeight = p1Height + heightFactor1;
			quadBuffer[i + j*size].b = p2sphere;
			quadBuffer[i + j*size].bHeight = p2Height + heightFactor2;
			quadBuffer[i + j*size].c = p3sphere;
			quadBuffer[i + j*size].cHeight = p3Height + heightFactor3;
			quadBuffer[i + j*size].d = p4sphere;
			quadBuffer[i + j*size].dHeight = p4Height + heightFactor4;
			quadBuffer[i + j*size].drawDistance = drawDistance;
			quadBuffer[i + j*size].heightFactor = heightFactor;

		}
	}

	return quadBuffer;
}


T_QUADNODE *  generateSphereMesh( float radius, unsigned int nbSlices, Texture * textureHeight) {
	int quadCount = nbSlices * nbSlices;
	T_QUADNODE * quadBuffer = (T_QUADNODE *)malloc(sizeof(T_QUADNODE) * quadCount);

	for (int i = 0;i < quadCount;i++) {
		quadBuffer[i].down = NULL;
		quadBuffer[i].left = NULL;
		quadBuffer[i].renderable = NULL;
		quadBuffer[i].right = NULL;
		quadBuffer[i].up = NULL;
		quadBuffer[i].vertexBuffer = NULL;
		quadBuffer[i].sublodList = NULL;
		quadBuffer[i].indexBuffer = NULL;
	}

	unsigned int count = 0;
	for (int i = 0;i < nbSlices;i++) {
		for (int j = 0;j < nbSlices;j++) {

			float alpha = glm::pi<float>()*1 / (nbSlices+1) * i;
            float beta = glm::pi<float>()*2 / nbSlices * j;
            float alpha1 = glm::pi<float>()*1  / (nbSlices+1) * (i + 1);
            float beta1 = glm::pi<float>()*2 / nbSlices * (j + 1);

			float v[] = { (float)i/nbSlices, (float)j/nbSlices, (float)(i+1)/nbSlices,  (float)(j+1)/nbSlices};
			

			float heightFactor = 200.f;
			glm::vec3 a = glm::vec3(radius * sin(alpha) * cos(beta), radius * cos(alpha),radius * sin(alpha) * sin(beta));
            glm::vec3 b = glm::vec3(radius * sin(alpha) * cos(beta1), radius * cos(alpha),radius * sin(alpha) * sin(beta1));
            glm::vec3 c = glm::vec3(radius * sin(alpha1) * cos(beta1), radius * cos(alpha1),radius * sin(alpha1) * sin(beta1));
            glm::vec3 d = glm::vec3(radius * sin(alpha1) * cos(beta), radius * cos(alpha1),radius * sin(alpha1) * sin(beta));

			int hmapSize = 16;
			
			unsigned int * indexBuffer;
			float * vertexBuffer = generateHeighmap(hmapSize, textureHeight, a, 0.f,d, 0.f,c, 0.f, b, 0.f, v[0], v[1], v[2], v[3], heightFactor, &indexBuffer, true, radius);

			quadBuffer[i + j*nbSlices].vertexBuffer = vertexBuffer; 
			quadBuffer[i + j*nbSlices].indexBuffer = indexBuffer;
			quadBuffer[i + j*nbSlices].triangleCount = hmapSize * hmapSize * 2;
			quadBuffer[i + j*nbSlices].position = (a + b + c + d) * 0.25f; 
			quadBuffer[i + j*nbSlices].normal = glm::normalize(quadBuffer[i + j*nbSlices].position );
			quadBuffer[i + j*nbSlices].a = a;
			quadBuffer[i + j*nbSlices].b = d;
			quadBuffer[i + j*nbSlices].c = c;
			quadBuffer[i + j*nbSlices].d = b;
			quadBuffer[i + j*nbSlices].aHeight = getHeightmapInterpolation(textureHeight,v[0],v[1]) * heightFactor;
			quadBuffer[i + j*nbSlices].bHeight = getHeightmapInterpolation(textureHeight,v[2],v[1]) * heightFactor;
			quadBuffer[i + j*nbSlices].cHeight = getHeightmapInterpolation(textureHeight,v[2],v[3]) * heightFactor;
			quadBuffer[i + j*nbSlices].dHeight = getHeightmapInterpolation(textureHeight,v[0],v[3]) * heightFactor;
			quadBuffer[i + j*nbSlices].drawDistance = 1500.f;
			quadBuffer[i + j*nbSlices].heightFactor = heightFactor;
		}
	}

	return quadBuffer;
}


void Renderer::initializeContent() {
	/* initialize random seed: */
	srand (time(NULL));
	float unit = -1.0f;
	unsigned int size = 16;

	g_textureHeight->blur();
	g_textureHeight->blur();
	g_textureHeight->blur();
	g_textureHeight->blur();

	float * vertexSphere = generateSphere(50000, size);

	
	g_SkySphere = Renderable::createRenderable(this->shaderTexturing, g_textureHeight, vertexSphere, size * size * 2);


	g_quadnodeList = generateSphereMesh(g_radius,size,g_textureHeight);

	for (int i = 0;i < size*size;i++) {
		T_QUADNODE * quad = &g_quadnodeList[i];
		g_quadnodeList[i].renderable = Renderable::createRenderable(this->shaderTexturing, g_textureHeight, quad->vertexBuffer, quad->triangleCount, quad->indexBuffer, 8);
	}

	g_asyncload = true;
}

static int AsyncInitThread(void *ptr) 
{
	Renderer * renderer = (Renderer*)ptr;
	renderer->initializeContent();
	return 0;
}


void Renderer::init(unsigned int screenWidth, unsigned int screenHeight, bool fullscreen)
{
	this->screenHeight = screenHeight;
	this->screenWidth = screenWidth;

	for (int i = 0;i < 10;i++) {
		effectList[i].duration = 0;
	}

	if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO| SDL_INIT_JOYSTICK) < 0 ) {
		fprintf(stderr, "Impossible d'initialiser SDL: %s\n", SDL_GetError());
		exit(1);
	}


    g_joystick = SDL_JoystickOpen(0);

    atexit(SDL_Quit);

    // Version d'OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);


	displayWindow = SDL_CreateWindow("Korridor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->screenWidth, this->screenHeight, SDL_WINDOW_OPENGL  | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));

    // Cr�ation du contexte OpenGL
  
    contexteOpenGL = SDL_GL_CreateContext(displayWindow);
  
    if(contexteOpenGL == 0)
    {
        std::cout << SDL_GetError() << std::endl; //// >> AFFICHE : " the specified window isn't an OpenGL window"
        SDL_DestroyWindow(displayWindow);
        SDL_Quit();
  
        exit(-1); //// >> PLANTE ICI : return -1 ..
    }
      

	if (TTF_Init() < 0) {
		puts("ERROR : unable to initialize font library");
		exit(1);
	}

	// GLEW Initialisation
	glewInit();

	SDL_RWops* fontdataptr = SDL_RWFromConstMem(test,sizeof(test));
	this->font = TTF_OpenFontRW(fontdataptr, 1, 28);
	//this->font = TTF_OpenFont( "digital display tfb.ttf",28);
	if (this->font == NULL)
	{
		puts("ERROR : unable to load font");
		exit(1);
	}


#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    unsigned int rmask = 0xff000000;
    unsigned int gmask = 0x00ff0000;
    unsigned int bmask = 0x0000ff00;
    unsigned int amask = 0x000000ff;
#else
    unsigned int rmask = 0x000000ff;
    unsigned int gmask = 0x0000ff00;
    unsigned int bmask = 0x00ff0000;
    unsigned int amask = 0xff000000;
#endif
	this->textSurface = SDL_CreateRGBSurface( 0, this->screenWidth, this->screenHeight, 32, rmask, gmask, bmask, amask);
	bExit = false;

	shaderTexturing = Shader::createBuiltin(SHADER_TEXTURING);

	Texture * textSurfaceTexture = new Texture(this->screenWidth,this->screenHeight,(unsigned char*)this->textSurface->pixels);
	this->spriteTextSurface = new Sprite(textSurfaceTexture,(float)this->screenWidth,(float)this->screenHeight,0,1,1,0);
	this->fbDrawing = NULL;

	g_shader_skybox = new Shader();
	g_shader_skybox->load_fragment_from_string(fragment_skybox);
	g_shader_skybox->load_vertex_from_string(vertex);
	
	g_shader_terrain = new Shader();
	g_shader_terrain->load_fragment_from_string(fragment_terrain);
	g_shader_terrain->load_vertex_from_string(vertex_terrain);

	camera = new Camera();
	camera->SetClipping(1.f,200000.f);
	camera->SetPosition(glm::vec3(-3000.0f,7000.0f,-3000.0f));
	camera->SetLookAt(glm::vec3(0.0f,0.0f,0.0f));

	OutputConsole::setRenderer(this);

	UIHeader * headWidget = new UIHeader();

	UIHeader * header3 = new UIHeader();
	header3->setLabel("Resume");
	header3->setOnClickCallback(&func_resume_cb,this);
	headWidget->addChild(header3);

	UIHeader * header1 = new UIHeader();
	header1->setLabel("Option");
	{
		UIHeader * headerOption1 = new UIHeader();
		headerOption1->setLabel("Joystick detection");
		headerOption1->setOnClickCallback(&func_joystick_cb,NULL);
		header1->addChild(headerOption1);

		UIBoolean * headerBoolean = new UIBoolean();
		headerBoolean->setLabel("Collision detection");
		headerBoolean->setOnBoolChangeCallback(func_bool_collision_change_cb,NULL);
		headerBoolean->setState(g_collision_detection);
		header1->addChild(headerBoolean);

		UIBoolean * headerBoolean2 = new UIBoolean();
		headerBoolean2->setLabel("Faces culling");
		headerBoolean2->setOnBoolChangeCallback(func_bool_cullface_change_cb,NULL);
		headerBoolean2->setState(g_cullface);	
		header1->addChild(headerBoolean2);

		UIBoolean * headerBoolean3 = new UIBoolean();
		headerBoolean3->setLabel("Post-processing");
		headerBoolean3->setOnBoolChangeCallback(func_bool_postprocess_change_cb,NULL);
		headerBoolean3->setState(g_postprocess);
		header1->addChild(headerBoolean3);

		UIHeader * headerOptionBack = new UIHeader();
		headerOptionBack->setLabel("Back");
		headerOptionBack->setOnClickCallback(&func_back_cb,headerOptionBack);
		header1->addChild(headerOptionBack);
	}

	headWidget->addChild(header1);
	UIHeader * header2 = new UIHeader();
	header2->setLabel("Quit");
	header2->setOnClickCallback(&func_exit_cb,this);
	headWidget->addChild(header2);

	UIWidget::currentWidget = headWidget;
	
	asyncInitThread = SDL_CreateThread(AsyncInitThread, "AsyncInitThread", (void *)this);
}


bool Renderer::exitstate()
{
	return bExit; 
}

void Renderer::setExitState() {
	bExit = true;
}

int thread_func(void *data)
{
	Renderer * renderer = (Renderer*)data;
    int last_value = 0;

    while ( renderer->exitstate() != true ) {
		renderer->draw();
    }
    printf("Thread quitting\n");
    return(0);
}


void Renderer::drawFps()
{
	char s[256];
	sprintf(s,"FPS %.3d",this->getFps());
	drawMessage(s,ALIGNRIGHT,ALIGNTOP);
}

void Renderer::drawMessage(const char * message,float x,float y) {
	if (strlen(message) == 0)
		return;

	SDL_Color fontcolor = {255, 255, 255};

	SDL_Surface * textSur = TTF_RenderText_Solid(this->font,message,fontcolor);	//set the text surface
	if (textSur == NULL)
	{
		
        fprintf(stderr, "Unable to create draw surface: %s\n", SDL_GetError());
		printf("%s\n",message);
        return;
	}

	drawMessage(textSur,x,y);
	SDL_FreeSurface(textSur);	
}

void Renderer::drawMessage(SDL_Surface * textSur,float x,float y) {
	SDL_Rect rect;
	rect.x = (int)x;
	rect.y = (int)y;
	SDL_BlitSurface(textSur,NULL,this->textSurface,&rect);
}

void Renderer::drawMessage(const char * message,RendererTextAlign hAlign,RendererTextAlign vAlign)
{
	
	SDL_Color fontcolor = {255, 255, 255};


	if (strlen(message) == 0)
		return;

	SDL_Surface * textSur = TTF_RenderText_Solid(this->font,message,fontcolor);	//set the text surface
	
	if (textSur == NULL)
	{
		
        fprintf(stderr, "Unable to create draw surface: %s\n", SDL_GetError());
		printf("%s\n",message);
        return;
	}
	SDL_Rect rect;
	float hudScale = 1.0f;

	switch (hAlign)
	{
	case ALIGNLEFT:
		rect.x = (int)(this->textSurface->w - this->textSurface->w* hudScale );
		break;
	case ALIGNRIGHT:
		rect.x = (int)(this->textSurface->w * hudScale - textSur->w) ;
		break;
	case ALIGNCENTER:
		rect.x = (int)(this->textSurface->w - textSur->w)/2;
		break;
	}

	switch (vAlign)
	{
	case ALIGNTOP:
		rect.y = (int)(this->textSurface->h - this->textSurface->h* hudScale );
		break;
	case ALIGNBOTTOM:
		rect.y = (int)(this->textSurface->h * hudScale - textSur->h) ;
		break;
	case ALIGNCENTER:
		rect.y = (int)(this->textSurface->h - textSur->h)/2;
		break;
	}
	
	drawMessage(textSur,rect.x,rect.y);
	
	SDL_FreeSurface(textSur);	
	
}

void swapFBdrawing(FrameBuffer **fb1, FrameBuffer **fb2) {
	FrameBuffer * fbtmp;
	fbtmp = *fb1;
	*fb1 = *fb2;
	*fb2 = fbtmp; 
}

void Renderer::drawScene(Shader * shader, FrameBuffer * frameBuffer) {
	if (frameBuffer != NULL) frameBuffer->bind();	

	glClearColor(0, 0, 0.1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	this->loopHook();

	shader->bind();

	for (unsigned int i = 0;i < g_renderableList.size() && g_asyncload;i++) {
		//glm::mat4 mv = g_renderableList[i]->model * this->camera->view; 
		//memcpy(shader->getModelViewMatrix(),glm::value_ptr(mv),sizeof(float)*16);
		//shader->bind_attributes();
		g_renderableList[i]->draw();	
	}
	
	shader->unbind();

	if (frameBuffer != NULL) frameBuffer->bind();
}

void Renderer::initDraw() {
	if (this->fbDrawing == NULL){
		this->fbDrawing = new FrameBuffer(this->screenWidth,this->screenHeight);
		this->fbDrawing->do_register();
	}
	
	//OpenGL setup
	if (g_cullface) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}

	glViewport((GLint) (0),
				(GLint) (0),
				(GLsizei) this->screenWidth,
				(GLsizei) this->screenHeight);

	camera->SetViewport((GLint) (0),
				(GLint) (0),
				(GLsizei) this->screenWidth,
				(GLsizei) this->screenHeight);
	
	glMatrixMode(GL_MODELVIEW);
}

void Renderer::drawUI() {
	this->fbDrawing->bind();
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	
	this->shaderTexturing->setProjectionMatrixToOrtho(this->screenWidth,this->screenHeight);
	this->shaderTexturing->setModelViewMatrixToIdentity();
	this->shaderTexturing->bind();
	
					
	char s[1024];
	glm::vec3 cam_pos = camera->getPosition();
	sprintf(s,"Pos: %.1f %.1f %.1f",cam_pos[0],cam_pos[1],cam_pos[2]);
	this->drawMessage(s,RendererTextAlign::ALIGNRIGHT,RendererTextAlign::ALIGNBOTTOM);
	
	if (UIWidget::currentWidget->isActive()){
		UIWidget::currentWidget->drawChilds(this);
	}
	
	
	this->drawFps();

	OutputConsole::render();

	glEnable(GL_BLEND);
	this->spriteTextSurface->updateTexture();
	this->spriteTextSurface->draw();
	glDisable(GL_BLEND);	
	this->shaderTexturing->unbind();	
}

void renderQuadNode(T_QUADNODE * quadNodeList, Camera * camera, Shader * shader) {
	if (quadNodeList != NULL) {
		for(int i = 0;i < 16 * 16;i++) {
			if (glm::dot(quadNodeList[i].normal,camera->camera_direction) <= 0.f) {

				if (glm::distance(quadNodeList[i].position, camera->camera_position) < quadNodeList[i].drawDistance) {
					///camera->far_clip = quadNodeList[i].drawDistance * 2.f;
					camera->speed = quadNodeList[i].drawDistance / 100.f;
					
					if (quadNodeList[i].sublodList == NULL) {
						quadNodeList[i].sublodList = generateQuadMesh(16, g_textureHeight, quadNodeList[i].a, quadNodeList[i].b, 
							quadNodeList[i].c, quadNodeList[i].d, 
							quadNodeList[i].aHeight,quadNodeList[i].bHeight,quadNodeList[i].cHeight,quadNodeList[i].dHeight,
								quadNodeList[i].drawDistance / 8.f, quadNodeList[i].heightFactor / 16.f,g_radius);
						quadNodeList[i].sublodSize = 16;
						for (int j = 0;j < 16*16;j++) {
							quadNodeList[i].sublodList[j].renderable = 
								Renderable::createRenderable(shader, g_textureHeight, quadNodeList[i].sublodList[j].vertexBuffer, 
								quadNodeList[i].sublodList[j].triangleCount, quadNodeList[i].sublodList[j].indexBuffer, 8);
						}
					} 
					renderQuadNode(quadNodeList[i].sublodList, camera, shader);
				} else {
					g_renderableList.push_back(quadNodeList[i].renderable);
				}
			}
		}	
	}
}


void Renderer::loopHook() {
	g_renderableList.clear();
	
	renderQuadNode(g_quadnodeList,this->camera, this->shaderTexturing);
	for (int i = 0;i < g_renderableList.size();i++) {
		g_renderableList.at(i)->draw();
	}
	
	g_renderableList.clear();
	
	g_shader_skybox->bind();
	g_shader_skybox->setProjectionAndModelViewMatrix(glm::value_ptr(camera->projection),glm::value_ptr(camera->view));
	g_shader_skybox->bind_custom_vector_attibute("u_CameraPosition",glm::value_ptr(glm::vec4(camera->getPosition(),0)));
	g_shader_skybox->bind_attributes();	
	
	if (g_SkySphere != NULL) g_SkySphere->draw();
	g_shader_skybox->unbind();
	
}


void Renderer::draw()
{
	SDL_FillRect(this->textSurface, NULL, 0x000000);

	camera->Update();
	glEnable(GL_DEPTH_TEST);
	this->shaderTexturing->setProjectionAndModelViewMatrix(glm::value_ptr(camera->projection),glm::value_ptr(camera->view));
	
	g_shader_terrain->bind();
	g_shader_terrain->setProjectionAndModelViewMatrix(glm::value_ptr(camera->projection),glm::value_ptr(camera->view));
	g_shader_terrain->bind_custom_vector_attibute("u_CameraPosition",glm::value_ptr(glm::vec4(camera->getPosition(),0)));
	g_shader_terrain->bind_attributes();
	drawScene(g_shader_terrain,this->fbDrawing);
	g_shader_terrain->unbind();

	this->drawUI();
	this->fbDrawing->unbind(screenWidth,screenHeight);

	this->fbDrawing->draw(screenWidth,screenHeight,this->shaderTexturing);
	SDL_GL_SwapWindow(displayWindow);
}

void Renderer::loop()
{
	SDL_Event event;
	T_EFFECT effect;
	int told = SDL_GetTicks();

	while (!bExit)
	{
		frameCounter++;
		if (frameCounter%10 == 0) {
			int tnew = SDL_GetTicks();
			int dt = tnew - told;
			told = tnew;

			if (dt != 0) {
				this->fps = 10000/dt;
			}
		}




		while( SDL_PollEvent( &event ) )
		{
			if (UIWidget::currentWidget->isActive()) {
				UIWidget::currentWidget->handleEvent(event);
			} else {
				camera->handleEvent(event);
				switch( event.type )
				{
					case SDL_JOYBUTTONDOWN:
						if (event.jbutton.button == 4) {
							SDL_SetRelativeMouseMode(SDL_FALSE);
							UIWidget::currentWidget->setActive(true);
						}
						break;
					case SDL_KEYDOWN:
						switch( event.key.keysym.sym )
						{
							case SDLK_ESCAPE:
								SDL_SetRelativeMouseMode(SDL_FALSE);
								UIWidget::currentWidget->setActive(true);
								break;
							case SDLK_p:
								this->fbDrawing->bind();
								this->fbDrawing->writeToTGA("fbDrawing.tga");
								this->fbDrawing->unbind(screenWidth,screenHeight);
								break;
							default:
								break;
						}
					break;
				}                
			}
		}
		
		this->draw();
	}

    
	SDL_FreeSurface(this->textSurface);
	SDL_GL_DeleteContext(contexteOpenGL);
    SDL_DestroyWindow(displayWindow);
    
	if (g_joystick != NULL) {
		SDL_JoystickClose(g_joystick);
	}
	SDL_Quit();
}

int Renderer::getFps()
{
	return this->fps;
}

void Renderer::processEffect(T_EFFECT &effect) {
	effect.duration--;
	switch (effect.effectType) {
	case 4 : {
				this->drawMessage(effect.str_param,effect.fl_param1,effect.fl_param2);
			 }
			 break;
	case 3 : {
			}
			break;
	case 2  :
			break;
	case 1 : 
			break;
	}
}

void Renderer::addEffect(T_EFFECT effect) {
	for(int i = 0;i < 10;i++) {
		if (effectList[i].duration == 0) {
			effectList[i].duration = effect.duration;
			effectList[i].effectType = effect.effectType;
			strcpy(effectList[i].str_param, effect.str_param);
			effectList[i].fl_param1 = effect.fl_param1;
			effectList[i].fl_param2 = effect.fl_param2;
			return;
		}
	}
}

Renderer::~Renderer() {

	//delete this->camera;
}