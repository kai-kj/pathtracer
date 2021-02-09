#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <CL/cl.h>

#define K_UTIL_IMPLEMENTATION
#include "../../include/k_util.h"

#include "renderer.h"

// testing
#define WIDTH 400
#define HEIGHT 300
#define SAMPLES 1000

// fast
// #define WIDTH 960
// #define HEIGHT 540
// #define SAMPLES 1000

// 1080p
// #define WIDTH 1920
// #define HEIGHT 1080
// #define SAMPLES 2000

// 4k
// #define WIDTH 3840
// #define HEIGHT 2160
// #define SAMPLES 10000

#define MAX_DEPTH 50
#define FOV PI / 3
#define WALL_RADIUS 10000

int main(void) {
	double start = get_time();

	Renderer *renderer = create_renderer();

	set_image_properties(renderer, WIDTH, HEIGHT, MAX_DEPTH);

	set_background_color(renderer, 0, 0, 0);

	Material lightSource = create_light_source_material(5, 5, 2.5);
	Material white = create_lambertian_material(1, 1, 1);
	Material red = create_lambertian_material(1, 0, 0);
	Material green = create_lambertian_material(0, 1, 0);
	Material mirror = create_metal_material(0, 0, 0, 0, 0);
	Material greenGlass = create_dielectric_material(0, 1, 0, 0.8, 0, 0.95);

	add_sphere(renderer, 0, 54.98, -10, 50, lightSource);

	add_sphere(renderer, 0, WALL_RADIUS + 5, -10, WALL_RADIUS, white);
	add_sphere(renderer, 0, -(WALL_RADIUS + 5), -10, WALL_RADIUS, white);
	add_sphere(renderer, 0, 0, -(WALL_RADIUS + 20), WALL_RADIUS, white);
	// add_sphere(renderer, 0, 0, WALL_RADIUS + 10, WALL_RADIUS, mirror);

	add_sphere(renderer, WALL_RADIUS + 5, 0, -5, WALL_RADIUS, green);
	add_sphere(renderer, -(WALL_RADIUS + 5), 0, -5, WALL_RADIUS, red);
	
	add_sphere(renderer, -2.5, -3, -10, 2, greenGlass);
	add_sphere(renderer, 2, -3.5, -15, 1.5, mirror);

	set_camera_properties(
		renderer,
		0, 0, 10,
		0, 0, 0,
		10, 20, 0.001, 1000
	);

	render_to_file(renderer, SAMPLES, "render.png", 1);

	destroy_renderer(renderer);

	double end = get_time();

	msg("\nFINISHED RENDER\n");

	float time = end - start;
	int h = sec_to_h(time);
	int min = sec_to_min(time) - h * 60;
	float sec = time - h * 3600 - min * 60;

	msg("[time]: %02d:%02d:%.2f (%f[fps])\n", h, min, sec, (float)SAMPLES / sec);

	return 0;
}
