#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <CL/cl.h>

#define PI 3.14159265358979323846

#define K_UTIL_IMPLEMENTATION
#include "../include/k_util.h"

#define K_IMAGE_IMPLEMENTATION
#include "../include/k_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"

#include "renderer/renderer_module.h"

int to_voxel_index(Scene scene, int x, int y, int z) {
	return x + y * scene.size.x + z * scene.size.x * scene.size.y;
}

int main(void) {
	Renderer *r = create_renderer();

	set_image_properties(r, 400, 300);

	set_background_color(r, 0, 0, 0);

	int width = 100;
	int height = 100;
	int depth = 100;

	r->scene.size = (cl_int3){.x = width, .y = height, .z = depth};
	r->scene.voxels = malloc(sizeof(cl_int) * width * height * depth);

	for(int i = 0; i < width * height * depth; i++) {
		r->scene.voxels[i] = 0;
	}

	int y = 0;
	for(int x = 0; x < width; x++) {
		for(int z = 0; z < depth; z++) {
			r->scene.voxels[to_voxel_index(r->scene, x, y, z)] = 1;
		}
	}

	int z = 1;
	for(int x = 0; x < width; x++) {
		for(int y = 0; y < depth; y++) {
			r->scene.voxels[x + y * width + z * width * height] = 1;
		}
	}

	set_camera_properties(r, 50, 20, 50, PI, 0, 0, 1, 1, 0.0011, 1000);

	render_to_file(r, 1, "render.png", 1);

	destroy_renderer(r);

	return 0;
}