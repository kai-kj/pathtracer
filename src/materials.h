#ifndef MATERIALS_H
#define MATERIALS_H

#include "../include/k_image.h"

typedef struct Material {
	int type;
	Color color;

	float reflectance;
	float fuzzyness;
	float refIdx;
} Material;

Material create_lambertian_material(Color color, float reflectance);
Material create_metal_material(Color color, float reflectance, float fuzzyness);
Material create_dielectric_material(Color color,  float refIdx, float fuzzyness);
Material create_light_source_material(Color color);

#endif