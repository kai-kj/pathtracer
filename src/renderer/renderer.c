#include <math.h>

#include "../../include/k_util.h"

#include "renderer.h"
#include "host/device_manager.h"

//---- private functions -----------------------------------------------------//

static void initialise_scene(Renderer *renderer) {
	renderer->scene = (Scene){
		(cl_int3){0, 0, 0},
		NULL,
		0,
		NULL,
		(cl_float3){0, 0, 0}
	};
}

//---- private functions -----------------------------------------------------//

Renderer *create_renderer() {
	Renderer *renderer = malloc(sizeof(Renderer));

	initialise_scene(renderer);

	create_kernel(renderer);

	return renderer;
}

void set_image_properties(Renderer *renderer, int width, int height) {
	renderer->image.size = (cl_int2){.x = width, .y = height};
	renderer->image.data = malloc(sizeof(cl_float3) * width * height);
	renderer->clProgram.imageBuff = clCreateBuffer(renderer->clProgram.context, CL_MEM_READ_WRITE, sizeof(cl_float3) * width * height, NULL, NULL);
}

void set_background_color(Renderer *renderer, float r, float g, float b) {
	renderer->scene.bgColor = (cl_float3){.x = r, .y = g, .z = b};
}

void set_camera_properties(Renderer *renderer, float x, float y, float z, float rotX, float rotY, float rotZ, float sensorWidth, float focalLength, float aperture, float exposure) {
	renderer->camera = (Camera){.sensorWidth = sensorWidth, .focalLength = focalLength, .aperture = aperture, .exposure = exposure};
	renderer->camera.pos = (cl_float3){.x = x, .y = y, .z = z};
	renderer->camera.rot = (cl_float3){.x = rotX, .y = rotY, .z = rotZ};
}

Image *render(Renderer *renderer, int samples, int verbose) {
	setup_kernel_args(renderer);

	run_kernel(renderer, samples);

	Image *image = get_image(renderer);

	return image;
}

void render_to_file(Renderer *renderer, int samples, char *fileName, int verbose) {
	Image *image = render(renderer, samples, verbose);

	write_image(image, fileName);
	
	destroy_image(image);
}

void destroy_renderer(Renderer *renderer) {
	release_kernel(renderer);
}