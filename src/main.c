#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

#define K_IMAGE_IMPLEMENTATION
#include "../include/k_image.h"

#define K_MATH_IMPLEMENTATION
#include "../include/k_math.h"

#define K_LOG_IMPLEMENTATION
#define K_LOG_PRINT_TO_SCREEN
#define K_LOG_PRINT_TO_FILE
#include "../include/k_log.h"

#include "materials.h"
#include "geometry.h"
#include "scene.h"
#include "load_scenes.h"

#define WIDTH 400
#define HEIGHT 300
#define FOV PI / 3
#define SAMPLES 1000
#define MAX_DEPTH 10

double get_time() {
	struct timeval time;
	gettimeofday(&time, NULL);

	return (double)time.tv_sec + (double)time.tv_usec / 1000000;
}

int sec_to_min(int seconds) {
	return seconds / 60;
}

int sec_to_h(int seconds) {
	return sec_to_min(seconds) / 60;
}


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

Color cast_ray(Scene *scene, Ray *ray, int depth) {
	Color color = scene->bgColor;

	if(depth > 0) {
		HitInfo *info = malloc(sizeof(HitInfo));

		if(hit_object(scene, ray, info)) {
			Vec3f offsettedHitPos = vec3f_add(info->pos, vec3f_mul_scalar(info->normal, -EPSILON));
			Color *self = &info->material->color;

			switch(info->material->type) {
				case 0: /* light source */ {
					color = *self;
					break;
				}

				case 1: /* lambertian material */ {
					Vec3f target = vec3f_add3(offsettedHitPos, info->normal, random_unit_vector());
					Color reflected = cast_ray(scene, &(Ray){offsettedHitPos, vec3f_sub(target, offsettedHitPos)}, depth - 1);
					float reflectance = info->material->reflectance;
					
					color = color_mul(color_mul_scalar(reflected, reflectance), color_mul_scalar(*self, 1 - reflectance));
					break;
				}

				case 2: /* metal material */ {
					Vec3f target = vec3f_add(reflection_dir(ray->direction, info->normal), vec3f_mul_scalar(random_unit_vector(), info->material->fuzzyness));
					Color reflected = cast_ray(scene, &(Ray){offsettedHitPos, target}, depth - 1);
					float reflectance = info->material->reflectance;
					
					color = color_add(color_mul_scalar(reflected, reflectance), color_mul_scalar(*self, 1 - reflectance));
					break;
				}

				case 3: /* dielectric material */ {
					float relativeRefIdx;

					if(info->front) /* entering material */
						relativeRefIdx = info->material->refIdx;
					else /* exiting material */
						relativeRefIdx = 1 / info->material->refIdx;
					
					double cosTheta = min_f(vec3f_dot(vec3f_mul_scalar(ray->direction, -1), info->normal), 1);
					double sinTheta = sqrt(1 - cosTheta * cosTheta);
					
					if (relativeRefIdx * sinTheta > 1 || reflectance(cosTheta, relativeRefIdx) > rand_float()) /* ray can't refract */ {
						Vec3f target = reflection_dir(ray->direction, info->normal);
						Color reflected = cast_ray(scene, &(Ray){offsettedHitPos, target}, depth - 1);
						color = reflected;
						break;

					} else  /* ray can refract */ {
						Vec3f target =refraction_dir(ray->direction, info->normal, relativeRefIdx);
						Color refracted = cast_ray(scene, &(Ray){offsettedHitPos, target}, depth - 1);
						color = refracted;
						break;
						
					}
				}
			}
		}

		free(info);
	}
	
	return color;
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

			Color color = cast_ray(scene, &ray, camera->maxDepth);
			*pixel = color_div_scalar(color_add(color_mul_scalar(*pixel, frame), color), frame + 1);
		}
	}
}

void render(Image *image, Scene *scene, Camera *camera, long startTime) {
	for(int i = 0; i < camera->samples; i++) {
		render_frame(scene, camera, image, i);

		float frameTime = (get_time() - startTime) / (i + 1);
		float et = (camera->samples - (i * 1)) * frameTime;
		int h = sec_to_h(et);
		int min = sec_to_min(et) - h * 60;
		int sec = et - h * 360 - min * 60;
		msg("%d/%d samples (%f%%), ET: %02d:%02d:%02d\n", i + 1, camera->samples, (float)(i + 1) / camera->samples * 100, h, min, sec);
	}

	msg("\nDONE\n");
}

int main(void) {
	Image *image = create_image(WIDTH, HEIGHT, (Color){0, 0, 0});
	Scene *scene = load_scene3();
	Camera camera = (Camera){FOV, SAMPLES, MAX_DEPTH};

	double startTime = get_time();

	// render image
	render(image, scene, &camera, startTime);
	
	double endTime = get_time();

	gamma_correct_image(image);
	write_image_stb(image, "test.png");

	destroy_scene(scene);
	destroy_image(image);

	msg("done (%g[s])\n", endTime - startTime);

	return 0;
}
