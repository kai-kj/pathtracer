#ifndef MATERIAL_H
#define MATERIAL_H

#include <CL/cl.h>

#include "material_structs.h"
#include "renderer_structs.h"

Material create_lambertian_material(
	float r, float g, float b
);

Material create_metal_material(
	float r, float g, float b,
	float tint,
	float fuzzyness
);

Material create_dielectric_material(
	float r, float g, float b,
	float tint,
	float fuzzyness,
	float refIdx
);

Material create_light_source_material(
	float r, float g, float b
);

int add_material(
	Renderer *renderer, Material material
);

#endif