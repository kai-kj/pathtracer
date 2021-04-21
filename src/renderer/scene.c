#include "scene.h"

//---- private ---------------------------------------------------------------//

static int out_of_scene(Renderer *renderer, int x, int y, int z) {
	return x < 0 || x >= renderer->scene.size.x || y < 0 || y >= renderer->scene.size.y || z < 0 || z >= renderer->scene.size.z;
}

//---- public ----------------------------------------------------------------//

void set_background_color(Renderer *renderer, float r, float g, float b) {
	renderer->scene.bgColor = (cl_float3){.x = r, .y = g, .z = b};
}

void create_scene(Renderer *renderer, int width, int height, int depth) {
	renderer->scene.materialCount = 0;

	renderer->scene.size = (cl_int3){.x = width, .y = height, .z = depth};
	renderer->scene.voxels = malloc(sizeof(cl_int) * width * height * depth);

	for(int i = 0; i < width * height * depth; i++) {
		renderer->scene.voxels[i] = -1;
	}
}

void destroy_scene(Renderer *renderer) {
	if(renderer->scene.voxels != NULL) {
		free(renderer->scene.voxels);
		renderer->scene.voxels = NULL;
	}

	if(renderer->scene.materials != NULL) {
		free(renderer->scene.materials);
		renderer->scene.materials = NULL;
	}

	renderer->scene.size = (cl_int3){.x = 0, .y = 0, .z = 0};
	renderer->scene.materialCount = 0;
}

void add_voxel(Renderer *renderer, int x, int y, int z, MaterialID materialID) {
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