#include "geometry.h"

#include <stdlib.h>
#include <math.h>

int hit_sphere(Ray *ray, Sphere *sphere, float *distance) {
    Vec3f oc = vec3f_sub(ray->origin, sphere->center);
    float a = vec3f_size_squared(ray->direction);
    float bHalf = vec3f_dot(oc, ray->direction);
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