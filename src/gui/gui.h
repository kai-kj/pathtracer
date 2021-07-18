#ifndef GUI_H
#define GUI_H

#include <SDL2/SDL.h> 
#include <SDL2/SDL_ttf.h>
#include "../renderer/renderer.h"
#include "structs.h"

typedef char GUIStatus;
#define GUI_SUCCESS 0
#define GUI_FAILURE -1

extern SDLState s;

GUIStatus create_window(int width, int height);
GUIStatus start_main_loop();

#endif