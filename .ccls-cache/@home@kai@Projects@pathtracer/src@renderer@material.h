#ifndef MATERIAL_H
#define MATERIAL_H

#include "renderer_structs.h"

Material create_light_source_material(
	float r, float g, float b,
	float brightness
);

Material create_lambertian_material(
	float r, float g, float b
);

Material create_metal_material(
	float r, float g, float b,
	float tint,
	float fuzz
);

Material create_dielectric_material(
	float r, float g, float b,
	float tint,
	float fuzz,
	float refIdx
);

void reset_materials(
	Renderer *renderer
);

MaterialID add_material(
	Renderer *renderer, Material material
);

#endif