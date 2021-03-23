#ifndef RENDERER_STRUCTS_H
#define RENDERER_STRUCTS_H

#include <CL/cl.h>
#include "host/device_manager_structs.h"
#include "material_structs.h"

typedef struct Scene {
	cl_int3 size;
	cl_int *voxels;
	cl_int materialCount;
	Material *materials;
	cl_float3 bgColor;
} Scene;

typedef struct Camera {
	cl_float3 pos;
	cl_float3 rot;
	cl_float sensorWidth;
	cl_float focalLength;
	cl_float aperture;
	cl_float exposure;
} Camera;

typedef struct Renderer {
	CLProgram clProgram;
	CLImage image;
	Scene scene;
	Camera camera;
} Renderer;

#endif