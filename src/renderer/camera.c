#include "renderer.h"

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