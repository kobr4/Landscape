#pragma once;

#define SHADER_TEXTURING 1
#define SHADER_BLUR_HORIZONTAL 2
#define SHADER_BLUR_VERTICAL 3

class Shader{
private :

	char ** f_string;
	int * f_string_length;
	int f_length;
	char ** v_string;
	int * v_string_length;
	int v_length;
	int vertexShaderHandle;
	int fragmentShaderHandle;
	unsigned int programHandle;
	
	float modelViewMatrix[16];	
	int modelViewMatrixHandle;
	float projectionMatrix[16];
	int projectionMatrixHandle;
	float  colorVector[4];
	int colorVectorHandle;
	
	static const char * blurVerticalFragmentString;
	static const char * blurVerticalVertexString;
	static const char * blurHorizontalFragmentString;
	static const char * blurHorizontalVertexString;
	static const char * texturingFragmentString;
	static const char * texturingVertexString;

	static Shader * blurVerticalShader;
	static Shader * blurHorizontalShader;
	static Shader * texturingShader;
public :

	static int vertexPositionHandle;
	static int texCoordHandle;
	static int texture0Handle;
	static int texture1Handle;
	Shader() {
		programHandle = 0;
		colorVector[0] = 1.0f;
		colorVector[1] = 1.0f;
		colorVector[2] = 1.0f;
		colorVector[3] = 1.0f;
	};
	void load_fragment(const char * filename);
	void load_vertex(const char * filename);
	void load_fragment_from_string(const char * fragmentstr);
	void load_vertex_from_string(const char * vertexstr);
	static Shader * createBuiltin(unsigned int shaderType);
	void bind();
	void unbind();
	void do_register();
	void bind_attributes();
	float * getModelViewMatrix() { return modelViewMatrix;};
	float * getProjectionMatrix() { return projectionMatrix;};
	float * getColorVector() {return colorVector;};
	void setProjectionMatrixToOrtho(unsigned int width, unsigned int height);
	void setModelViewMatrixToIdentity();
	void setProjectionAndModelViewMatrix(float * P,float * MV);
	void bind_custom_vector_attibute(char * name, const float * vector);
};