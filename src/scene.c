#include "scene.h"

#include <stdlib.h>
#include "../include/k_image.h"
#include "geometry.h"

Scene *init_scene(Color bgColor) {
	Scene *scene = malloc(sizeof(Scene));
	scene->bgColor = bgColor;
	scene->sphereCount = 0;
	return scene;
}

void destroy_scene(Scene *scene) {
	free(scene);
}

void add_sphere(Scene *scene, float x, float y, float z, float radius, Material material) {
	Sphere sphere = (Sphere){(Vec3f){x, y, z}, radius, material};

	scene->sphereCount++;
	scene->spheres = realloc(scene->spheres, sizeof(Sphere) * scene->sphereCount);
	scene->spheres[scene->sphereCount - 1] = sphere;
}

int hit_object(Scene *scene, Ray *ray, HitInfo *info) {
	int hit = 0;
	Sphere *sphere;

	for(int i = 0; i < scene->sphereCount; i++) {
		float distance;
		if(hit_sphere(ray, &scene->spheres[i], &distance)) {
			if(!hit || distance < info->distance) {
				hit = 1;
				info->distance = distance;
				sphere =  &scene->spheres[i];
			}
		}
	}

	if(!hit)
		return 0;

	info->pos = vec3f_add(ray->origin, vec3f_mul_scalar(ray->direction, info->distance));
	info->normal = vec3f_normalise(vec3f_sub(info->pos, sphere->center));
	info->material = &sphere->material;

	if(vec3f_dot(ray->direction, info->normal) > 0) {
		info->front = 0;
		info->normal = vec3f_mul_scalar(info->normal, -1);
	
	} else {
		info->front = 1;
	}

	return 1;
}