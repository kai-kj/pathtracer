#include "voxel.h"

//---- private functions -----------------------------------------------------//

int out_of_scene(Renderer *renderer, int x, int y, int z) {
	return x < 0 || x >= renderer->scene.size.x || y < 0 || y >= renderer->scene.size.y || z < 0 || z >= renderer->scene.size.z;
}

//---- private functions -----------------------------------------------------//

void add_voxel(Renderer *renderer, int x, int y, int z, int materialID) {
	if(!out_of_scene(renderer, x, y, z)) {
		int index = x + y * renderer->scene.size.x + z * renderer->scene.size.x * renderer->scene.size.y;
		renderer->scene.voxels[index] = materialID;
	}
}

void remove_voxel(Renderer *renderer, int x, int y, int z) {
	if(!out_of_scene(renderer, x, y, z)) {
		int index = x + y * renderer->scene.size.x + z * renderer->scene.size.x * renderer->scene.size.y;
		renderer->scene.voxels[index] = 0;
	}
}