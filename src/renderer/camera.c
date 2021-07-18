#include "renderer.h"

RendererStatus set_camera_properties(
	float x, float y, float z,
	float rotX, float rotY, float rotZ,
	float sensorWidth, float focalLength, float aperture, float exposure
) {
	r.camera = (Camera){
		.sensorWidth = sensorWidth,
		.focalLength = focalLength,
		.aperture = aperture,
		.exposure = exposure
	};
	r.camera.pos = (cl_float3){.x = x, .y = y, .z = z};
	r.camera.rot = (cl_float3){.x = rotX, .y = rotY, .z = rotZ};

	return RENDERER_SUCCESS;
}