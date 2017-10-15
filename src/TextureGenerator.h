#pragma once

typedef struct {
	float position[3];
	float color[3];
} T_TextureLightSource;

typedef struct {
	float p1[3];
	float p2[3];
	float p3[3];
	float p4[3];
} T_TextureQuad;
typedef enum {
	TOP,
	BOTTOM,
	VERTICAL
} T_POSITION;

typedef enum {
	ENABLED,
	DISABLED
} T_STATE;

typedef struct {
	float p1[3];
	float p2[3];
	float p3[3];
	float p4[3];
	float pos_x;
	float pos_y;
	float distance;
	T_POSITION position;
	T_STATE state;
} T_QUAD;


class Texture;
class TextureGenerator {
public :
	static Texture * generateTileTexture(unsigned int width,unsigned int height, unsigned int hoffset,unsigned int hspacing,unsigned int vspacing,
		unsigned int border,unsigned char r, unsigned char g, unsigned char b,unsigned char jr, unsigned char jg, unsigned char jb);
	static Texture * generateWhiteTexture();
	static Texture * generateLightmapTexture(unsigned int width,unsigned int height,T_TextureLightSource * source, T_TextureQuad * quad);
	static Texture * generateLightmapTextureWithOcclusion(unsigned int width,unsigned int height,T_TextureLightSource * source, T_TextureQuad * quad,T_QUAD * quadOccluded, unsigned int quadArraySize,unsigned int quadToIgnore);
	static Texture * generateFloorLightmap(unsigned int width,unsigned int height,unsigned char value);
	static Texture * generatePerlinNoise(unsigned int width,unsigned int height);
	static Texture * generatePerlinNoiseGreyscale(unsigned int width,unsigned int height);
	static Texture * generateNormalMap(unsigned int width,unsigned int height, unsigned char * pixels);
	static void writeTGA(char * filename, unsigned char * map, int width, int height, bool invertRB);
};