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

#include "renderer/renderer.h"

int to_voxel_index(Scene scene, int x, int y, int z) {
	return x + y * scene.size.x + z * scene.size.x * scene.size.y;
}

int main(void) {
	Renderer *r = create_renderer();

	set_image_properties(r, 800, 600);

	int size = 10;
	int samples = 50;

	int width = size;
	int height = size;
	int depth = size;

	create_scene(r, width + 1, height + 1, depth + 1);

	// set_background_color(r, 1, 1, 1);
	set_background_color(r, 0, 0, 0);

	MaterialID white = add_material(r, create_lambertian_material(1, 1, 1));
	MaterialID red = add_material(r, create_lambertian_material(1, 0, 0));
	MaterialID green = add_material(r, create_lambertian_material(0, 1, 0));
	MaterialID blue = add_material(r, create_lambertian_material(0, 0, 1));
	MaterialID mirror = add_material(r, create_metal_material(0, 0, 0, 0.5, 0));
	MaterialID light = add_material(r, create_light_source_material(1, 1, 0.5, 3));

	// side walls
	for(int y = 0; y <= width; ++y) {
		for(int z = 0; z <= depth; ++z) {
			add_voxel(r, 0, y, z, red);
			add_voxel(r, width, y, z, mirror);
		}
	}

	// front wall
	for(int x = 0; x <= width; x++) {
		for(int y = 0; y <= height; y++) {
			add_voxel(r, x, y, 0, white);
		}
	}

	// back wall
	for(int x = 0; x <= width; x++) {
		for(int y = 0; y <= height; y++) {
			add_voxel(r, x, y, depth, white);
		}
	}

	// celling
	for(int x = 0; x <= width; x++) {
		for(int z = 0; z <= depth; z++) {
			add_voxel(r, x, 0, z, white);
		}
	}

	// floor
	for(int x = 0; x <= width; x++) {
		for(int z = 0; z <= depth; z++) {
			add_voxel(r, x, height, z, white);
		}
	}

	// light
	// for(int x = width * 4 / 10; x <= width * 6 / 10; x++) {
	// 	for(int z = depth * 4 / 10; z <= depth * 6 / 10; z++) {
	// 		add_voxel(r, x, 0, z, light);
	// 	}
	// }

	// objects
	// add_voxel(r, width * 0.4, height - 1, depth * 0.6, mirror);
	// add_voxel(r, width * 0.4, height - 2, depth * 0.6, mirror);

	add_voxel(r, width * 0.8, height - 1, depth * 0.7, light);
	add_voxel(r, width * 0.8, height - 2, depth * 0.7, light);
	// add_voxel(r, width * 0.9, height - 1, depth * 0.7, light);

	// set_camera_properties(r, size * 0.5, size * 0.4, 1, -PI / 16, 0, 0, 1, 1, 0.001, 1000);
	set_camera_properties(r, size * 0.5, size * 0.5, 1, 0, 0, 0, 1.5, 1, 0.001, 1000);

	render_to_file(r, samples, "render.png", 1);

	destroy_renderer(r);

	return 0;
}
