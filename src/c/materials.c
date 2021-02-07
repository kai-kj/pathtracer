#include "materials.h"

Material create_lambertian_material(Color color, float reflectance) {
	return (Material){1, rgb_to_float3(color), reflectance, 0, 0};
}

Material create_metal_material(Color color,  float reflectance, float fuzzyness) {
	return (Material){2, rgb_to_float3(color), reflectance, fuzzyness, 0};
}

Material create_dielectric_material(Color color,  float refIdx, float fuzzyness) {
	return (Material){3, rgb_to_float3(color), 0, fuzzyness, refIdx};
}

Material create_light_source_material(Color color) {
	return (Material){0, rgb_to_float3(color), 0, 0, 0};
}