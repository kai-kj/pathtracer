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

int main(void) {
	Renderer *r = create_renderer();

	set_image_properties(r, 400, 300, 50);

	set_background_color(r, 0, 0, 0);

	int width = 10;
	int height = 10;
	int depth = 10;

	r->voxels = malloc(sizeof(cl_int) * width * height * depth);

	for(int i = 0; i < width * height * depth; i++) {
		r->voxels[i] = 1;
	}

	for(int x = 3; x < 7; x++) {
		int y = 5;
		for(int z = 3; z < 7; z++) {
			r->voxels[x + y * width + z * width * height] = 1;
		}
	}

	r->sceneInfo.size = (cl_int3){.x = width, .y = height, .z = depth};

	set_camera_properties(r, 5, 0, 0, 0, 0, 0, 10, 20, 0.001, 1000);

	render_to_file(r, 1, "render.png", 1);

	destroy_renderer(r);

	return 0;
}