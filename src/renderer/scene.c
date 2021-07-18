#include "renderer.h"

//---- private ---------------------------------------------------------------//

static int _voxel_exists(int x, int y, int z) {
	for(int i = 0; i < r.scene.voxelCount; i++) {
		Voxel box = r.scene.voxels[i];
		if(box.pos.x == x && box.pos.y == y && box.pos.z == z)  return 1;
	}

	return 0;
}

static int _get_voxel_index(int x, int y, int z) {
	for(int i = 0; i < r.scene.voxelCount; i++) {
		Voxel box = r.scene.voxels[i];
		if(box.pos.x == x && box.pos.y == y && box.pos.z == z) return i;
	}

	return -1;
}

//---- public ----------------------------------------------------------------//

RendererStatus set_background_properties(float red, float green, float blue, float brightness) {
	r.scene.bgColor = (cl_float3){.x = red, .y = green, .z = blue};
	r.scene.bgBrightness = brightness;

	return RENDERER_SUCCESS;
}

RendererStatus destroy_scene() {
	r.scene.voxelCount = 0;

	if(r.scene.voxels != NULL) {
		free(r.scene.voxels);
		r.scene.voxels = NULL;
	}

	return RENDERER_SUCCESS;
}

RendererStatus remove_voxel(int x, int y, int z) {
	int idx = _get_voxel_index(x, y, z);

	if(idx == -1) return RENDERER_FAILURE;

	r.scene.voxelCount--;

	for(int i = idx; i < r.scene.voxelCount; i++) {
		r.scene.voxels[i] = r.scene.voxels[i + 1];
	}
	
	r.scene.voxels = realloc(r.scene.voxels, sizeof(Voxel) * r.scene.voxelCount);

	return RENDERER_SUCCESS;
}

RendererStatus add_voxel(int x, int y, int z, Material material) {
	if(_voxel_exists(x, y, z)) remove_voxel(x, y, z);

	Voxel box = (Voxel){
		(cl_int3){x, y, z},
		material
	};

	r.scene.voxelCount++;
	r.scene.voxels = realloc(r.scene.voxels, r.scene.voxelCount * sizeof(Voxel));

	r.scene.voxels[r.scene.voxelCount - 1] = box;

	return RENDERER_SUCCESS;
}