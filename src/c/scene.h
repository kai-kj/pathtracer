#ifndef SCENE_H
#define SCENE_H

#include <stdlib.h>
#include <CL/cl.h>

#include "materials.h"

typedef struct Sphere {
	cl_float3 center;
	cl_float radius;
	Material material;

} Sphere;

typedef struct Scene {
	cl_float3 bgColor;
	cl_int sphereCount;
} Scene;

typedef struct Camera {
	cl_int width;
	cl_int height;
	cl_float fov;
	cl_int samples;
	cl_int maxDepth;
} Camera;

int add_sphere(Sphere *sphereList, int sphereCount, float x, float y, float z, float radius, Material material);

#endif