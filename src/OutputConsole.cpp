#include "OutputConsole.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>  
#include "Renderer.h"

OutputConsole * OutputConsole::outputConsole = new OutputConsole();

OutputConsole::OutputConsole()  {
	this->logToFile = false;
	this->renderer = NULL;
	for (int i = 0; i < 4;i++) {
		this->sBuffer[i][0] = 0;
	}

	this->sToatBuffer[0] = 0;
}

void OutputConsole::setRenderer(Renderer * renderer) {
	OutputConsole::outputConsole->renderer = renderer;	
}

void OutputConsole::log(const char * message,...) {
	char s[1024];
	s[0] = 0;
	va_list argptr;
	va_start(argptr, message);
	vsprintf(s, message, argptr);
	va_end(argptr);	
	for (int i = 1;i < 4;i++) {
		strcpy(OutputConsole::outputConsole->sBuffer[i-1],OutputConsole::outputConsole->sBuffer[i]); 
	}
	strcpy(OutputConsole::outputConsole->sBuffer[3],s); 
	printf("%s",s);
}

void OutputConsole::render() {
	for (int i = 0;i < 4;i++) {
		OutputConsole::outputConsole->renderer->drawMessage(OutputConsole::outputConsole->sBuffer[i],0, i * 20);
	}

	if (SDL_GetTicks() < OutputConsole::outputConsole->toastDelay) {
		OutputConsole::outputConsole->renderer->drawMessage(OutputConsole::outputConsole->sToatBuffer,RendererTextAlign::ALIGNLEFT,RendererTextAlign::ALIGNBOTTOM);
	}
}

void OutputConsole::toast(unsigned int delayms,const char * message,...) {
	char s[1024];
	s[0] = 0;
	va_list argptr;
	va_start(argptr, message);
	vsprintf(s, message, argptr);
	va_end(argptr);	
	strcpy(OutputConsole::outputConsole->sToatBuffer,s);
	OutputConsole::outputConsole->toastDelay = SDL_GetTicks() + delayms;
}