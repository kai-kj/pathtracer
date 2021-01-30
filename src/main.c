#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>

#define K_IMAGE_IMPLEMENTATION
#include "../include/k_image.h"

#define K_MATH_IMPLEMENTATION
#include "../include/k_math.h"

#include "materials.h"
#include "geometry.h"
#include "scene.h"

Vec3f random_unit_vector() {
    while(1) {
        Vec3f r = (Vec3f){rand_float_in_range(-1, 1), rand_float_in_range(-1, 1), rand_float_in_range(-1, 1)};
        if(vec3f_size(r) < 1)
			return vec3f_normalise(r);
    }
}

Vec3f reflection_dir(Vec3f in, Vec3f normal) {
	return vec3f_sub(in, vec3f_mul_scalar(normal, 2 * vec3f_dot(in, normal)));
}

float reflectance(float cosTheta, float relativeRefIdx) {
	float r = (1 - relativeRefIdx) / (1 + relativeRefIdx);
	r = r * r;
	return r + (1 - r) * pow((1 - cosTheta), 5);
}

Vec3f refraction_dir(Vec3f in, Vec3f normal, float relativeRefIdx) {
    float cosTheta = min_f(vec3f_dot(vec3f_mul_scalar(in, -1), normal), 1);
    Vec3f outPerpendicular = vec3f_mul_scalar((vec3f_add(in, vec3f_mul_scalar(normal, cosTheta))), relativeRefIdx);
    Vec3f outParallel = vec3f_mul_scalar(normal, -sqrt(fabs(1 - vec3f_size_squared(outPerpendicular))));
    return vec3f_add(outParallel, outPerpendicular);
}

Color cast_ray(Scene *scene, Ray ray) {
	HitInfo *info = malloc(sizeof(HitInfo));

	if(hit_object(scene, ray, info)) {
		Vec3f offsettedHitPos = vec3f_add(info->pos, vec3f_mul_scalar(info->normal, -EPSILON));

		switch(info->material->type) {
			case 0: /* light source */ {
				return info->material->color;
			}

			case 1: /* lambertian material */ {
				Vec3f target = vec3f_add3(offsettedHitPos, info->normal, random_unit_vector());
				Color reflected = cast_ray(scene, (Ray){offsettedHitPos, vec3f_sub(target, offsettedHitPos)});
				Color self = info->material->color;
				float reflectance = info->material->reflectance;
				
				return color_mul(color_mul_scalar(reflected, reflectance), color_mul_scalar(self, 1 - reflectance)); 
			}

			case 2: /* metal material */ {
				Vec3f target = vec3f_add(reflection_dir(ray.direction, info->normal), vec3f_mul_scalar(random_unit_vector(), info->material->fuzzyness));
				Color reflected = cast_ray(scene, (Ray){offsettedHitPos, target});
				Color self = info->material->color;
				float reflectance = info->material->reflectance;
				
				return color_add(color_mul_scalar(reflected, reflectance), color_mul_scalar(self, 1 - reflectance));
			}

			case 3: /* dielectric material */ {
				float relativeRefIdx;
				if(!info->front)
					relativeRefIdx = 1 / info->material->refIdx;
				else
					relativeRefIdx = info->material->refIdx;
				
				double cosTheta = min_f(vec3f_dot(vec3f_mul_scalar(ray.direction, -1), info->normal), 1);
            	double sinTheta = sqrt(1 - cosTheta * cosTheta);
				
				if (relativeRefIdx * sinTheta > 1 || reflectance(cosTheta, relativeRefIdx) > rand_float()) {
					Vec3f target = reflection_dir(ray.direction, info->normal);
					Color reflected = cast_ray(scene, (Ray){offsettedHitPos, target});
					return(reflected);

				} else {
					Vec3f target =refraction_dir(ray.direction, info->normal, relativeRefIdx);
					Color refracted = cast_ray(scene, (Ray){offsettedHitPos, target});
					return(refracted);
					
				}
			}
		}
	}

	return scene->bgColor;
}

void render_frame(Scene *scene, Camera *camera, Image *image, int frame) {
	float tanFOV = tan(camera->fov / 2);
	float aspectRatio = (float)image->width / (float)image->height;

	for(int row = 0; row < image->height; row++) {
		for(int column = 0; column < image->width; column++) {
			Color *pixel = &image->data[column + row * image->width];

			float y = - (2 * (row + rand_float()) / image->height - 1) * tanFOV;
			float x = (2 * (column + rand_float()) / image->width - 1) * tanFOV * aspectRatio;

			Ray ray = (Ray){(Vec3f){0, 0, 0}, vec3f_normalise((Vec3f){x, y, -1})};

			Color color = cast_ray(scene, ray);
			*pixel = color_div_scalar(color_add(color_mul_scalar(*pixel, frame), color), frame + 1);
		}
	}
}

void render(Scene *scene, Camera *camera, Image *image) {
	for(int i = 0; i < camera->samples; i++) {
		render_frame(scene, camera, image, i);
		printf("%d/%d samples (%f%%)\r", i + 1, camera->samples, (float)(i + 1) / camera->samples * 100);
		fflush(stdout);
	}

	printf("\nDONE\n");
}

#define WIDTH 400
#define HEIGHT 300
#define FOV PI / 3
#define SAMPLES 50

#include "scenes.h"

int main(void) {
	struct timeval A, B;

	Image *image = create_image(WIDTH, HEIGHT, (Color){0, 0, 0});

	// scene
	Scene *scene = load_scene2();
	
	// camera
	Camera camera = (Camera){PI / 3, SAMPLES};

	// render image
	gettimeofday(&A, NULL);

	render(scene, &camera, image);

	gettimeofday(&B, NULL);

	// write image
	gamma_correct_image(image);
	write_image(image, "test.ppm");

	destroy_image(image);

	printf("done (%f[s])\n", (float)(B.tv_sec - A.tv_sec) + (float)(B.tv_usec - A.tv_usec) / 1000000);

	return 0;
}