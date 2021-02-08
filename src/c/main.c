#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <CL/cl.h>

#define k_cl_image_IMPLEMENTATION
#include "../../include/k_cl_image.h"

#define K_MATH_IMPLEMENTATION
#include "../../include/k_math.h"

#define K_UTIL_IMPLEMENTATION
#define K_LOG_PRINT_TO_SCREEN
#define K_LOG_PRINT_TO_FILE
#define K_LOG_FILE "log.txt"
#include "../../include/k_util.h"

#include "renderer.h"

// testing
#define WIDTH 400
#define HEIGHT 300
#define SAMPLES 100

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
	double startTime = get_time();

	//---- initialise values -------------------------------------------------//
	int pixelCount = WIDTH * HEIGHT;

	SceneInfo scene = {
		.sphereCount = 0,
		.bgColor.x = 0,
		.bgColor.y = 0,
		.bgColor.z = 0
	};

	Camera camera = {
		// .pos = {.x = 0, .y = 0, .z = 10},
		// .rot = {.x = 0, .y = 0, .z = 0},
		.pos = {.x = 2, .y = 2, .z = -15},
		.rot = {.x = -PI / 2, .y = 0, .z = 0},
		// .pos = {.x = 2, .y = -3, .z = -10},
		// .rot = {.x = 0, .y = 0, .z = 0},
		.resolution = {.x = WIDTH, .y = HEIGHT},
		.useDOF = 0,
		.sensorWidth = 0.05,
		.focalLength = 0.1,
		.aperture = 0.001,
		.exposure = 1000,
		.samples = SAMPLES,
		.maxRayDepth = MAX_DEPTH
	};

	Sphere *sphereList = malloc(sizeof(Sphere));

	Material lightSource = create_light_source_material((Color){5, 5, 2.5});
	Material white = create_lambertian_material((Color){1, 1, 1});
	Material red = create_lambertian_material((Color){1, 0, 0});
	Material green = create_lambertian_material((Color){0, 1, 0});
	Material mirror = create_metal_material((Color){0, 0, 0}, 0, 0);
	Material greenGlass = create_dielectric_material((Color){0, 1, 0}, 0.8, 0, 0.95);

	add_sphere(sphereList, &scene.sphereCount, 0, 54.98, -10, 50, lightSource);

	add_sphere(sphereList, &scene.sphereCount, 0, WALL_RADIUS + 5, -10, WALL_RADIUS, white);
	add_sphere(sphereList, &scene.sphereCount, 0, -(WALL_RADIUS + 5), -10, WALL_RADIUS, white);
	add_sphere(sphereList, &scene.sphereCount, 0, 0, -(WALL_RADIUS + 20), WALL_RADIUS, white);
	// add_sphere(sphereList, &scene.sphereCount, 0, 0, WALL_RADIUS + 10, WALL_RADIUS, mirror);

	add_sphere(sphereList, &scene.sphereCount, WALL_RADIUS + 5, 0, -5, WALL_RADIUS, green);
	add_sphere(sphereList, &scene.sphereCount, -(WALL_RADIUS + 5), 0, -5, WALL_RADIUS, red);
	
	add_sphere(sphereList, &scene.sphereCount, -2.5, -3, -10, 2, greenGlass);
	add_sphere(sphereList, &scene.sphereCount, 2, -3.5, -15, 1.5, mirror);

	//---- render ------------------------------------------------------------//

	time_t renderStartTime = get_time();

	cl_float3 *image = render(scene, camera, sphereList, 1);

	time_t renderEndTime = get_time();

	//---- write image -------------------------------------------------------//
	
	gamma_correct_image(image, pixelCount);
	write_image(image, WIDTH, HEIGHT, "test.png");

	//---- cleanup -----------------------------------------------------------//

	free(sphereList);
	free(image);

	//---- printf info -------------------------------------------------------//

	double endTime = get_time();

	msg("\nFINISHED RENDER\n");

	float time = renderEndTime - renderStartTime;
	int h = sec_to_h(time);
	int min = sec_to_min(time) - h * 60;
	float sec = time - h * 3600 - min * 60;

	msg("[render time]: %02d:%02d:%.2f (%f[fps])\n", h, min, sec, camera.samples / sec);

	time = endTime - startTime;
	h = sec_to_h(time);
	min = sec_to_min(time) - h * 60;
	sec = time - h * 3600 - min * 60;

	msg("[total time]: %02d:%02d:%.2f\n\n", h, min, sec);

	return 0;
}
