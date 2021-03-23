#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <CL/cl.h>

#include "../renderer_structs.h"
#include "../../../include/k_image.h"

void create_kernel(Renderer *renderer);

void setup_kernel_args(Renderer *renderer);

void run_kernel(Renderer *renderer, int samples);

Image *get_image(Renderer *renderer);

void release_kernel(Renderer *renderer);

#endif