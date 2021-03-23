#ifndef VOXEL_H
#define VOXEL_H

#include "renderer_structs.h"
#include "material_structs.h"

void add_voxel(
	Renderer *renderer,
	int x, int y, int z,
	int materialID
);

void remove_voxel(
	Renderer *renderer,
	int x, int y, int z
);

#endif