#ifndef KERNEL_H
#define KERNEL_H

#include <CL/cl.h>

#include "../../include/k_image.h"
#include "renderer.h"

Renderer *create_renderer();

void set_image_properties(
	Renderer *renderer,
	int width, int height
);

Image *render_image(
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
