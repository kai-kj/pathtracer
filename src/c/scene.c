#include "scene.h"

int add_sphere(Sphere *sphereList, int sphereCount, float x, float y, float z, float radius, Material material) {
	cl_float3 center = {.x = x, .y = y, .z = z};

	Sphere sphere = (Sphere){center, radius, material};
	sphereCount++;

	sphereList = realloc(sphereList, sizeof(Sphere) * sphereCount);
	sphereList[sphereCount - 1] = sphere;

	return sphereCount;
}

