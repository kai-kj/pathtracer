#include "renderer.h"

Material create_light_source_material(float r, float g, float b, float brightness) {
	Material m;

	m.type = 1;
	m.color = (cl_float3){r, g, b};
	m.details.lightSource.brightness = brightness;
	
	return m;
}

Material create_lambertian_material(float r, float g, float b) {
	Material m;

	m.type = 2;
	m.color = (cl_float3){r, g, b};

	return m;
}

Material create_metal_material(float r, float g, float b, float tint, float fuzz) {
	Material m;

	m.type = 3;
	m.color = (cl_float3){r, g, b};
	m.details.metal.tint = tint;
	m.details.metal.fuzz = fuzz;

	return m;
}

Material create_dielectric_material(float r, float g, float b, float tint, float fuzz, float refIdx) {
	Material m;

	m.type = 3;
	m.color = (cl_float3){r, g, b};
	m.details.dielectric.tint = tint;
	m.details.dielectric.fuzz = fuzz;
	m.details.dielectric.refIdx = refIdx;

	return m;
}