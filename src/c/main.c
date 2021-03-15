#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <CL/cl.h>

#define PI 3.14159265358979323846

#define K_UTIL_IMPLEMENTATION
#include "../../include/k_util.h"
#include "renderer.h"

int to_voxel_index(Scene scene, int x, int y, int z) {
	return x + y * scene.size.x + z * scene.size.x * scene.size.y;
}

int main(void) {
	Renderer *r = create_renderer();

	set_image_properties(r, 400, 300);

	set_background_color(r, 0, 0, 0);

	int width = 10;
	int height = 10;
	int depth = 10;

	r->scene.size = (cl_int3){.x = width, .y = height, .z = depth};
	r->scene.data = malloc(sizeof(cl_int) * width * height * depth);

	for(int i = 0; i < width * height * depth; i++) {
		r->scene.data[i] = 0;
	}

	int y = 0;
	for(int x = 0; x < width; x++) {
		for(int z = 0; z < depth; z++) {
			r->scene.data[to_voxel_index(r->scene, x, y, z)] = 1;
		}
	}

	// int z = 1;
	// for(int x = 0; x < width; x++) {
	// 	for(int y = 0; y < depth; y++) {
	// 		r->scene.data[x + y * width + z * width * height] = 1;
	// 	}
	// }

	set_camera_properties(r, 5, 2, 5, 0, 0, 0, 1, 1, 0.0011, 1000);

	render_to_file(r, 1, "render.png", 1);

	destroy_renderer(r);

	return 0;
}