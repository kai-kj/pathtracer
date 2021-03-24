#ifndef CAMERA_H
#define CAMERA_H

#include "renderer_structs.h"

void set_camera_properties(
	Renderer *renderer,
	float x, float y, float z,
	float rotX, float rotY, float rotZ,
	float sensorWidth, float focalLength, float aperture, float exposure
);

#endif