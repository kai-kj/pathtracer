#include <CL/cl.h>

#include "material.h"

Material create_light_source_material(float r, float g, float b) {
	return (Material){1, (cl_float3){.x = r, .y = g, .z = b}, 0, 0, 0};
}

Material create_lambertian_material(float r, float g, float b) {
	return (Material){2, (cl_float3){.x = r, .y = g, .z = b}, 0, 0, 0};
}

Material create_metal_material(float r, float g, float b,  float tint, float fuzzyness) {
	return (Material){3, (cl_float3){.x = r, .y = g, .z = b}, tint, fuzzyness, 0};
}

Material create_dielectric_material(float r, float g, float b, float tint, float fuzzyness, float refIdx) {
	return (Material){4, (cl_float3){.x = r, .y = g, .z = b}, tint, fuzzyness, refIdx};
}

void reset_materials(Renderer *renderer) {
	renderer->scene.materialCount = 0;

	if(renderer->scene.materials != NULL) {
		free(renderer->scene.materials);
		renderer->scene.materials = NULL;
	}
}

MaterialID add_material(Renderer *renderer, Material material) {
	renderer->scene.materialCount++;
	renderer->scene.materials = realloc(renderer->scene.materials, sizeof(Material) * renderer->scene.materialCount);

	renderer->scene.materials[renderer->scene.materialCount - 1] = material;

	return renderer->scene.materialCount - 1;
}