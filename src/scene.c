#include "scene.h"

Scene *init_scene(Color bgColor) {
	Scene *scene = malloc(sizeof(Scene));
	scene->bgColor = bgColor;
	scene->sphereCount = 0;
	return scene;
}

void destroy_scene(Scene *scene) {
	free(scene);
}

void add_sphere(Scene *scene, Sphere sphere) {
	scene->sphereCount++;
	scene->spheres = realloc(scene->spheres, sizeof(Sphere) * scene->sphereCount);
	scene->spheres[scene->sphereCount - 1] = sphere;
}