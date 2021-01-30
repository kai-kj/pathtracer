#include "geometry.h"

typedef struct Scene Scene;

Scene *load_scene1() {
	Material lightSource = create_light_source_material((Color){1, 1, 1});
	Material lightSourceGreen = create_light_source_material((Color){0, 1, 0});
	Material lambertianRed = create_lambertian_material((Color){1, 0, 0}, 0.5);
	Material lambertianWhite = create_lambertian_material((Color){1, 1, 1}, 0.5);
	Material mirror = create_metal_material((Color){0, 0, 0}, 1, 0);
	Material fuzzyMirror = create_metal_material((Color){0, 0, 0}, 1, 0.5);
	Material glass = create_dielectric_material((Color){0, 0, 0}, 0.8, 0);

	Scene *scene = init_scene((Color){0, 0, 0});

	add_sphere(scene, (Sphere){(Vec3f){0, -101, -8}, 100, lambertianWhite});

	add_sphere(scene, (Sphere){(Vec3f){1.5, 1, -8}, 1, lightSource});
	add_sphere(scene, (Sphere){(Vec3f){1.5, 0, -4}, 0.5, lightSourceGreen});
	add_sphere(scene, (Sphere){(Vec3f){-1.5, 0, -6}, 1, glass});

	return scene;
}

Scene *load_scene2() {
	Material lightSource = create_light_source_material((Color){1, 1, 1});
	Material lightSourceGreen = create_light_source_material((Color){0, 1, 0});
	Material lambertianRed = create_lambertian_material((Color){1, 0, 0}, 0.5);
	Material lambertianWhite = create_lambertian_material((Color){1, 1, 1}, 0.5);
	Material mirror = create_metal_material((Color){0, 0, 0}, 1, 0);
	Material fuzzyMirror = create_metal_material((Color){0, 0, 0}, 1, 0.5);
	Material glass = create_dielectric_material((Color){0, 0, 0}, 0.8, 0);

	Scene *scene = init_scene((Color){0.5, 0.7, 1});

	add_sphere(scene, (Sphere){(Vec3f){-10, 10, 0}, 5, lightSource});
	add_sphere(scene, (Sphere){(Vec3f){0, -101, -8}, 100, lambertianWhite});

	add_sphere(scene, (Sphere){(Vec3f){-2, 0, -5}, 1, lambertianRed});
	add_sphere(scene, (Sphere){(Vec3f){0, 0, -10}, 1, lightSourceGreen});
	add_sphere(scene, (Sphere){(Vec3f){2, 0, -8}, 1, mirror});
	add_sphere(scene, (Sphere){(Vec3f){3, 0, -5}, 1, glass});

	return scene;
}