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

	set_image_properties(r, 400, 300);
	
	int width = 100;
	int height = 100;
	int depth = 100;

	create_scene(r, width + 1, height + 1, depth + 1);
	set_background_color(r, 0, 0, 0);

	MaterialID white_wall = add_material(r, create_lambertian_material(1, 1, 1));
	MaterialID red_wall = add_material(r, create_lambertian_material(1, 0, 0));
	MaterialID green_wall = add_material(r, create_lambertian_material(0, 1, 0));
	MaterialID light = add_material(r, create_light_source_material(10, 10, 8));

	// side walls
	for(int y = 0; y < width; y++) {
		for(int z = 0; z < depth; z++) {
			add_voxel(r, 0, y, z, red_wall);
			add_voxel(r, width, y, z, green_wall);
		}
	}

	// back wall
	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {
			add_voxel(r, x, y, depth, white_wall);
		}
	}

	// floor / celling
	for(int x = 0; x < width; x++) {
		for(int z = 0; z < depth; z++) {
			add_voxel(r, x, 0, z, white_wall);
			add_voxel(r, x, height, z, white_wall);
		}
	}

	// light
	for(int x = width / 4; x < width / 4 * 3; x++) {
		for(int z = depth / 4; z < depth / 4 * 3; z++) {
			add_voxel(r, x, 0, z, light);
		}
	}

	set_camera_properties(r, 50, 50, 1, 0, 0, 0, 1, 1, 0.001, 1000);

	render_to_file(r, 100, "render.png", 1);

	destroy_renderer(r);

	return 0;
}