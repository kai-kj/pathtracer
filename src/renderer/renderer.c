#include <stdlib.h>
#include <stdio.h>
#include <CL/cl.h>

#include "renderer.h"

//---- scene -----------------------------------------------------------------//

void set_background_color(Renderer *r, float red, float green, float blue) {
	r->scene.bgColor = (cl_float3){.x = red, .y = green, .z = blue};
}

void create_scene(Renderer *r, int width, int height, int depth) {
	r->scene.boxCount = 0;
	r->scene.boxes = malloc(sizeof(AABB));
}

void destroy_scene(Renderer *r) {
	r->scene.boxCount = 0;

	if(r->scene.boxes != NULL) {
		free(r->scene.boxes);
		r->scene.boxes = NULL;
	}
}

void add_voxel(Renderer *r, int x, int y, int z, Material material) {
	AABB box = (AABB){
		(cl_float3){x, y, z},
		(cl_float3){x + 1, y + 1, z + 1},
		material
	};

	r->scene.boxCount++;
	r->scene.boxes = realloc(r->scene.boxes, r->scene.boxCount * sizeof(AABB));

	r->scene.boxes[r->scene.boxCount - 1] = box;
}


//---- camera ----------------------------------------------------------------//

void set_camera_properties(
	Renderer *renderer,
	float x, float y, float z,
	float rotX, float rotY, float rotZ,
	float sensorWidth, float focalLength, float aperture, float exposure
) {
	renderer->camera = (Camera){
		.sensorWidth = sensorWidth,
		.focalLength = focalLength,
		.aperture = aperture,
		.exposure = exposure
	};
	renderer->camera.pos = (cl_float3){.x = x, .y = y, .z = z};
	renderer->camera.rot = (cl_float3){.x = rotX, .y = rotY, .z = rotZ};
}

//---- material --------------------------------------------------------------//

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

