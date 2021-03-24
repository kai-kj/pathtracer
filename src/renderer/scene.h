#ifndef SCENE_H
#define SCENE_H

#include "renderer_structs.h"

void set_background_color(Renderer *renderer, float r, float g, float b);

void create_scene(Renderer *renderer, int width, int height, int depth);

void destroy_scene(Renderer *renderer);

void add_voxel(Renderer *renderer, int x, int y, int z, MaterialID materialID);

void remove_voxel(Renderer *renderer, int x, int y, int z);

#endif