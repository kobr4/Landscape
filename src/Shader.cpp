#include "Shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OutputConsole.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int Shader::vertexPositionHandle = 1;
int Shader::texCoordHandle = 2;
int Shader::texture0Handle = 0;
int Shader::texture1Handle = 1;

Shader * Shader::blurHorizontalShader = NULL;
Shader * Shader::blurVerticalShader = NULL;
Shader * Shader::texturingShader= NULL;

const char * Shader::blurVerticalFragmentString =
"\
varying vec4 vColor;\
varying vec2 v_texture_coordinate;\
uniform sampler2D my_color_texture;\
uniform sampler2D my_lightmap_texture;\
float offset = 0.004;\
\
void main(void) {\
	gl_FragColor = vColor * (\
	texture2D(my_color_texture, v_texture_coordinate)\
	+ texture2D(my_color_texture, v_texture_coordinate - vec2(0.0,offset))\
	+ texture2D(my_color_texture, v_texture_coordinate + vec2(0.0,offset))\
	+ texture2D(my_color_texture, v_texture_coordinate - vec2(0.0,offset*2))\
	+ texture2D(my_color_texture, v_texture_coordinate + vec2(0.0,offset*2))\
	) / 5.0;\
}";
const char * Shader::blurVerticalVertexString =
"\
uniform mat4 u_MVMatrix;\
uniform mat4 u_PMatrix;\
uniform vec4 u_CVector;\
varying vec4 vColor;\
varying vec2 v_texture_coordinate;\
attribute vec4 a_Position;\
attribute vec2 a_TexCoord;\
\
void main(void) {\
	vColor = u_CVector;\
	gl_Position = u_PMatrix * u_MVMatrix * a_Position;\
	v_texture_coordinate = a_TexCoord;\
}";
const char * Shader::blurHorizontalFragmentString =
"\
varying vec4 vColor;\
varying vec2 v_texture_coordinate;\
uniform sampler2D my_color_texture;\
uniform sampler2D my_lightmap_texture;\
float offset = 0.004;\
\
void main(void) {\
	gl_FragColor = vColor * (\
	texture2D(my_color_texture, v_texture_coordinate)\
	+ texture2D(my_color_texture, v_texture_coordinate - vec2(offset,0.0))\
	+ texture2D(my_color_texture, v_texture_coordinate + vec2(offset,0.0))\
	+ texture2D(my_color_texture, v_texture_coordinate - vec2(offset*2,0.0))\
	+ texture2D(my_color_texture, v_texture_coordinate + vec2(offset*2,0.0))\
	) / 5.0;\
}";
const char * Shader::blurHorizontalVertexString =
"\
uniform mat4 u_MVMatrix;\
uniform mat4 u_PMatrix;\
uniform vec4 u_CVector;\
varying vec4 vColor;\
varying vec2 v_texture_coordinate;\
attribute vec4 a_Position;\
attribute vec2 a_TexCoord;\
\
void main(void) {\
	vColor = u_CVector;\
	gl_Position = u_PMatrix * u_MVMatrix * a_Position;\
	v_texture_coordinate = a_TexCoord;\
}";

const char * Shader::texturingFragmentString =
"\
varying vec4 vColor;\
varying vec2 v_texture_coordinate;\
uniform sampler2D my_color_texture;\
uniform sampler2D my_lightmap_texture;\
\
void main(void) {\
	gl_FragColor = vColor * texture2D(my_color_texture, v_texture_coordinate);\
}";

const char * Shader::texturingVertexString =
"\
uniform mat4 u_MVMatrix;\
uniform mat4 u_PMatrix;\
uniform vec4 u_CVector;\
varying vec4 vColor;\
varying vec2 v_texture_coordinate;\
attribute vec4 a_Position;\
attribute vec2 a_TexCoord;\
\
void main(void) {\
	vColor = u_CVector;\
	gl_Position = u_PMatrix * u_MVMatrix * a_Position;\
	v_texture_coordinate = a_TexCoord;\
}";
void load_from_string(const char * s_string,int &length,char **&string, int *&stringlength) {
	unsigned int s_len = strlen(s_string);
	unsigned int nbline = 0;
	for (unsigned int i = 0;i <= s_len;i++) {
		if (s_string[i] == 0 || s_string[i] == '\n') {
			nbline++;
		}
	}

	string = (char**)malloc(sizeof(char*)*nbline);
	stringlength = (int*)malloc(sizeof(int)*nbline);
	length = nbline;
	unsigned int counter = 0;
	for (unsigned int i = 0;i < nbline;i++) {
		unsigned int line_length = 0;
		unsigned int line_start = counter;
		
		for (;s_string[counter] != '\n' && s_string[counter] != 0;counter++) {
			line_length++;
		}
		counter = line_start;

		string[i] = (char*)malloc(sizeof(char)*(line_length+1));
		stringlength[i] = line_length+1;
		for (;s_string[counter] != '\n' && s_string[counter] != 0;counter++) {
			string[i][counter - line_start] = s_string[counter];
		}

		string[i][line_length] = 0;
	}

}

void load_string(const char * filename,int &length,char **&string, int *&stringlength) {
	char s[2048];
	FILE * f;
	f = fopen(filename,"r");
	if (f == NULL) {
		printf("Couldn't load file %s\n",filename);
		exit(0); 
	}

	length = 0;
	while (fgets (s, 2048, f)!=NULL)
	length++;
	fseek(f,0,SEEK_SET); 
	string = (char **)malloc(sizeof(char*)*length);
	stringlength = (int *)malloc(sizeof(int)*length);
	length = 0;
	while (fgets (s, 2048, f)!=NULL){
		int l = strlen(s) + 1;
		string[length] = (char*)malloc(sizeof(char)*l);
		strcpy(string[length],s);
		stringlength[length] = strlen(string[length]);
		length++;
	}

	fclose(f);
}

Shader * Shader::createBuiltin(unsigned int shaderType) {
	switch(shaderType) {
		case SHADER_TEXTURING:
			if (Shader::texturingShader != NULL) {
				return Shader::texturingShader;
			}

			Shader::texturingShader = new Shader();
			load_from_string(texturingFragmentString,Shader::texturingShader->f_length,Shader::texturingShader->f_string,Shader::texturingShader->f_string_length);
			load_from_string(texturingVertexString,Shader::texturingShader->v_length,Shader::texturingShader->v_string,Shader::texturingShader->v_string_length);
			return Shader::texturingShader;
		case SHADER_BLUR_HORIZONTAL:
			if (Shader::blurHorizontalShader != NULL) {
				return Shader::blurHorizontalShader;
			}

			Shader::blurHorizontalShader = new Shader();
			load_from_string(blurHorizontalFragmentString,Shader::blurHorizontalShader->f_length,Shader::blurHorizontalShader->f_string,Shader::blurHorizontalShader->f_string_length);
			load_from_string(blurHorizontalVertexString,Shader::blurHorizontalShader->v_length,Shader::blurHorizontalShader->v_string,Shader::blurHorizontalShader->v_string_length);
			
			return Shader::blurHorizontalShader;
		case SHADER_BLUR_VERTICAL:
			if (Shader::blurVerticalShader != NULL) {
				return Shader::blurVerticalShader;
			}

			Shader::blurVerticalShader = new Shader();
			load_from_string(blurVerticalFragmentString,Shader::blurVerticalShader->f_length,Shader::blurVerticalShader->f_string,Shader::blurVerticalShader->f_string_length);
			load_from_string(blurVerticalVertexString,Shader::blurVerticalShader->v_length,Shader::blurVerticalShader->v_string,Shader::blurVerticalShader->v_string_length);
			return Shader::blurVerticalShader;
		default : 
			OutputConsole::log("Cannot create built-in shader. TYPE=%d\n",shaderType);
			return NULL;
	}
}

void Shader::load_fragment_from_string(const char * fragmentstr) {
	load_from_string(fragmentstr,this->f_length,this->f_string,this->f_string_length);
}

void Shader::load_vertex_from_string(const char * vertexstr) {
	load_from_string(vertexstr,this->v_length,this->v_string,this->v_string_length);
}

void Shader::load_fragment(const char * filename) {
	load_string(filename,this->f_length,this->f_string,this->f_string_length);
}

void Shader::load_vertex(const char * filename) {
	load_string(filename,this->v_length,this->v_string,this->v_string_length);
}

void Shader::bind_attributes() {
	glUniformMatrix4fv(this->modelViewMatrixHandle, 1, 0,this->modelViewMatrix);
	glUniformMatrix4fv(this->projectionMatrixHandle, 1, 0,this->projectionMatrix);
	glUniform4fv(this->colorVectorHandle,1,this->colorVector);
	//glUniform1i(this->texture0Handle,0);
	glUniform1i(this->texture1Handle,1);
}

void Shader::bind_custom_vector_attibute(char * name, float * vector) {
	int ulocation = glGetUniformLocation(this->programHandle, name);
	glUniform4fv(ulocation, 1, vector);
}

void Shader::bind()
{
	if (programHandle == 0) {
		do_register();
	}
    glUseProgram(programHandle);



	this->bind_attributes();

}

void Shader::unbind()
{
    glUseProgram(0);			
}

void Shader::do_register() {

	GLint status;
	char buffer[2048];
	int length;
	
	glewExperimental=GL_TRUE;
	glewInit();

	this->vertexShaderHandle = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(this->vertexShaderHandle, this->v_length,(const GLchar **)this->v_string,this->v_string_length);
	glCompileShader(this->vertexShaderHandle);

	glGetShaderiv(this->vertexShaderHandle,GL_COMPILE_STATUS,&status);
	if (status == GL_FALSE) {
		glGetShaderInfoLog(this->vertexShaderHandle,2048,&length,buffer);
		puts("Fatal error on vertex shader compilation.");
		puts(buffer);
		exit(0);
	}

	this->fragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(this->fragmentShaderHandle, this->f_length,(const GLchar **)this->f_string,this->f_string_length);
	glCompileShader(this->fragmentShaderHandle);

	glGetShaderiv(this->vertexShaderHandle,GL_COMPILE_STATUS,&status);
	if (status == GL_FALSE) {
		glGetShaderInfoLog(this->vertexShaderHandle,2048,&length,buffer);
		puts("Fatal error on fragment shader compilation.");
		puts(buffer);
		exit(0);
	}

	this->programHandle = glCreateProgram();
	glAttachShader(this->programHandle, this->vertexShaderHandle);
	glAttachShader(this->programHandle, this->fragmentShaderHandle);

	glBindAttribLocation(this->programHandle, Shader::vertexPositionHandle, "a_Position");
	glBindAttribLocation(this->programHandle, Shader::texCoordHandle, "a_TexCoord");

	glLinkProgram(this->programHandle);

	this->modelViewMatrixHandle = glGetUniformLocation(this->programHandle, "u_MVMatrix");
	this->projectionMatrixHandle = glGetUniformLocation(this->programHandle, "u_PMatrix");
	this->colorVectorHandle = glGetUniformLocation(this->programHandle, "u_CVector");
	
	this->texture0Handle = glGetUniformLocation(this->programHandle, "my_color_texture");
	this->texture1Handle = glGetUniformLocation(this->programHandle, "my_lightmap_texture");
}

void Shader::setProjectionMatrixToOrtho(unsigned int width, unsigned int height) {
	glm::mat4 projMat = glm::ortho( 0.f, (float)width, (float)height, 0.f, -1.f, 1.f );
	memcpy(this->getProjectionMatrix(), glm::value_ptr(projMat),sizeof(float)*16);
}

void Shader::setModelViewMatrixToIdentity() {
	memcpy(this->getModelViewMatrix(),glm::value_ptr(glm::mat4()),sizeof(float)*16);
}

void Shader::setProjectionAndModelViewMatrix(float * p,float * mv) {
	memcpy(this->getProjectionMatrix(),p,sizeof(float)*16);
	memcpy(this->getModelViewMatrix(),mv,sizeof(float)*16);
}