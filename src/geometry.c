#include "geometry.h"

int hit_sphere(Ray ray, Sphere *sphere, float *distance) {
    Vec3f oc = vec3f_sub(ray.origin, sphere->center);
    float a = vec3f_size_squared(ray.direction);
    float bHalf = vec3f_dot(oc, ray.direction);
    float c = oc.x * oc.x + oc.y * oc.y + oc.z * oc.z - sphere->radius * sphere->radius;
    float discriminant = bHalf *bHalf - a * c;

	if(discriminant < 0)
		return 0;
	
	*distance = (-bHalf - sqrt(discriminant)) / a;

	if(*distance > 0)
		return 1;
	else
		return 0;
}

int hit_object(Scene *scene, Ray ray, HitInfo *info) {
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

	info->pos = vec3f_add(ray.origin, vec3f_mul_scalar(ray.direction, info->distance));
	info->normal = vec3f_normalise(vec3f_sub(info->pos, sphere->center));
	info->material = &sphere->material;

	if(vec3f_dot(ray.direction, info->normal) > 0) {
		info->front = 0;
		info->normal = vec3f_mul_scalar(info->normal, -1);
	
	} else {
		info->front = 1;
	}

	return 1;
}