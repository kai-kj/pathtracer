#ifndef SCENE_H
#define SCENE_H

#include "geometry.h"

typedef struct Scene {
	Color bgColor;
	Sphere *spheres;
	int sphereCount;
} Scene;

typedef struct Camera {
	float fov;
	int samples;
} Camera;

Scene *init_scene(Color bgColor);
void destroy_scene(Scene *scene);
void add_sphere(Scene *scene, Sphere sphere);
int hit_object(Scene *scene, Ray *ray, HitInfo *info);

#endif