#define GLEW_STATIC
#include "Renderer.h"
#include <stdlib.h>
#include <stdio.h>
#undef main
int main(int argc, char **argv) {
	unsigned int w = 1920;
	unsigned int h = 1080;
	bool fullscreen = false;
	
	for (int i = 0; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'f':
				fullscreen = true;
				break;
			case 'h':
				h = atoi(&argv[i][2]);
				if (h == 0) {
					puts("Invalid height(-h) parameter.");
				}
				break;
			case 'w':
				w = atoi(&argv[i][2]);
				if (w == 0) {
					puts("Invalid width(-w) parameter.");
				}
				break;
			}
		}
	}
	
	
	Renderer renderer = Renderer();
	renderer.init(w, h, fullscreen);
	renderer.initDraw();
	renderer.loop();
	
	return 0;
}