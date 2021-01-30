#ifndef SCENE_H
#define SCENE_H

#include <stdlib.h>
#include "../include/k_image.h"
#include "geometry.h"

typedef struct Sphere Sphere;

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

#endif