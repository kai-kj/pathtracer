#include "renderer.h"

//---- private ---------------------------------------------------------------//

static int _voxel_exists(Renderer *r, int x, int y, int z) {
	for(int i = 0; i < r->scene.voxelCount; i++) {
		Voxel box = r->scene.voxels[i];
		if(box.pos.x == x && box.pos.y == y && box.pos.z == z)  return 1;
	}

	return 0;
}

static int _get_voxel_index(Renderer *r, int x, int y, int z) {
	for(int i = 0; i < r->scene.voxelCount; i++) {
		Voxel box = r->scene.voxels[i];
		if(box.pos.x == x && box.pos.y == y && box.pos.z == z) return i;
	}

	return -1;
}

//---- public ----------------------------------------------------------------//

void set_background_color(Renderer *r, float red, float green, float blue) {
	r->scene.bgColor = (cl_float3){.x = red, .y = green, .z = blue};
}

void create_scene(Renderer *r, int width, int height, int depth) {
	r->scene.voxelCount = 0;
	r->scene.voxels = malloc(sizeof(Voxel));
}

void destroy_scene(Renderer *r) {
	r->scene.voxelCount = 0;

	if(r->scene.voxels != NULL) {
		free(r->scene.voxels);
		r->scene.voxels = NULL;
	}
}

void remove_voxel(Renderer *r, int x, int y, int z) {
	int idx = _get_voxel_index(r, x, y, z);

	if(idx != -1) {
		r->scene.voxelCount--;

		for(int i = idx; i < r->scene.voxelCount; i++) {
			r->scene.voxels[i] = r->scene.voxels[i + 1];
		}
		
		r->scene.voxels = realloc(r->scene.voxels, sizeof(Voxel) * r->scene.voxelCount);
	}
}

void add_voxel(Renderer *r, int x, int y, int z, Material material) {
	if(_voxel_exists(r, x, y, z)) remove_voxel(r, x, y, z);

	Voxel box = (Voxel){
		(cl_int3){x, y, z},
		material
	};

	r->scene.voxelCount++;
	r->scene.voxels = realloc(r->scene.voxels, r->scene.voxelCount * sizeof(Voxel));

	r->scene.voxels[r->scene.voxelCount - 1] = box;
}