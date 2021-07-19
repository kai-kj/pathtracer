#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <CL/cl.h>

#define PI 3.14159265358979323846

#define K_UTIL_IMPLEMENTATION
#include <k_tools/k_util.h>

#define K_IMAGE_IMPLEMENTATION
#include <k_tools/k_image.h>

// #define K_OPENCL_IMPLEMENTATION
// #include "k_tools/k_opencl.h"

#include "renderer/renderer.h"
#include "gui/gui.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

int main(void) {
	create_renderer();

	set_image_properties(800, 600);

	int size = 10;
	int samples = 100;

	int width = size;
	int height = size;
	int depth = size;

	set_background_properties(0.5, 0.5, 1, 0.1);

	Material white = create_lambertian_material(0.8, 0.8, 0.8);
	Material red = create_lambertian_material(1, 0, 0);
	Material green = create_lambertian_material(0, 1, 0);
	Material blue = create_lambertian_material(0, 0, 1);
	Material mirror = create_metal_material(0, 0, 0, 0.5, 0);
	Material light = create_light_source_material(1, 1, 0.5, 2);
	Material sun = create_light_source_material(1, 1, 0.8, 10);

	// side walls
	for(int y = 0; y <= width; ++y) {
		for(int z = 0; z <= depth; ++z) {
			add_voxel(0, y, z, red);
			add_voxel(width, y, z, green);
		}
	}

	// front wall
	// for(int x = 0; x <= width; x++) {
	// 	for(int y = 0; y <= height; y++) {
	// 		add_voxel(x, y, 0, white);
	// 	}
	// }

	// back wall
	for(int x = 0; x <= width; x++) {
		for(int y = 0; y <= height; y++) {
			add_voxel(x, y, depth, blue);
		}
	}

	// celling
	for(int x = 0; x <= width; x++) {
		for(int z = 0; z <= depth; z++) {
			add_voxel(x, 0, z, white);
		}
	}

	// floor
	for(int x = 0; x <= width; x++) {
		for(int z = 0; z <= depth; z++) {
			add_voxel(x, height, z, white);
		}
	}

	// objects
	add_voxel(width * 0.7, height - 1, depth * 0.5, light);
	add_voxel(width * 0.7, height - 2, depth * 0.5, light);

	add_voxel(width * 0.3, height - 1, depth * 0.3, white);
	add_voxel(width * 0.3, height - 2, depth * 0.3, white);

	add_voxel(width * 0.3, height - 1, depth * 0.8, mirror);
	add_voxel(width * 0.3, height - 2, depth * 0.8, mirror);
	add_voxel(width * 0.4, height - 1, depth * 0.8, mirror);
	add_voxel(width * 0.4, height - 2, depth * 0.8, mirror);
	add_voxel(width * 0.5, height - 1, depth * 0.8, mirror);
	add_voxel(width * 0.5, height - 2, depth * 0.8, mirror);
	add_voxel(width * 0.6, height - 1, depth * 0.8, mirror);
	add_voxel(width * 0.6, height - 2, depth * 0.8, mirror);
	add_voxel(width * 0.7, height - 1, depth * 0.8, mirror);
	add_voxel(width * 0.7, height - 2, depth * 0.8, mirror);

	for(int x = -5; x <= 15; x++) {
		for(int z = -5; z <= 15; z++) {
			add_voxel(x, -height * 2, z - 5, sun);
		}
	}

	set_camera_properties(
		5.5, -2, -10,
		-M_PI / 8, 0, 0,
		1, 1,
		0.001, 1000
	);

	
	if(create_window(SCREEN_WIDTH, SCREEN_HEIGHT) == GUI_SUCCESS)
		start_main_loop();
	
	close_window();

	// render_image_to_file(samples, "render.png");

	destroy_renderer();

	return 0;
}


/*

renderer
	renderer.h

	renderer.c
	scene.c
	camera.c
	material.c
	image.c

*/