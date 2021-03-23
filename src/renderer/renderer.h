#ifndef RENDERER_H
#define RENDERER_H

#include "renderer_structs.h"
#include "../../include/k_image.h"

Renderer *create_renderer();

void set_image_properties(
	Renderer *renderer,
	int width, int height
);

void set_background_color(
	Renderer *renderer,
	float r, float g, float b
);

void set_camera_properties(
	Renderer *renderer,
	float x, float y, float z,
	float rotX, float rotY, float rotZ,
	float sensorWidth,
	float focalLength,
	float aperture,
	float exposure
);

Image *render(
	Renderer *renderer,
	int samples,
	int verbose
);

void render_to_file(
	Renderer *renderer,
	int samples,
	char *fileName,
	int verbose
);

void destroy_renderer(
	Renderer *renderer
);

#endif