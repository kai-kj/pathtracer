#ifndef GUI_STRUCTS_H
#define GUI_STRUCTS_H

#include "gui.h"

typedef struct SDLState {
	int quit;
	float dt;
	unsigned long int prevFrameTime;
	SDL_Window *window;
	SDL_Surface *windowSurface;
	SDL_Surface *renderSurface;
	TTF_Font *font;

} SDLState;

#endif