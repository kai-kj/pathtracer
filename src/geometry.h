#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "../include/k_math.h"
#include "materials.h"

typedef struct Ray {
	Vec3f origin;
	Vec3f direction;
} Ray;

typedef struct Sphere {
	Vec3f center;
	float radius;
	Material material;
} Sphere;

typedef struct HitInfo {
	int front;
	Vec3f pos;
	Vec3f normal;
	float distance;
	Material *material;
} HitInfo;

int hit_sphere(Ray *ray, Sphere *sphere, float *distance);

#endif