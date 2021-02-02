#include "geometry.h"
#include "scene.h"

Scene *load_scene1() {
	Scene *scene = init_scene((Color){0, 0, 0});

	Material lightSource = create_light_source_material((Color){1, 1, 1});
	Material lightSourceGreen = create_light_source_material((Color){0, 1, 0});
	Material lambertianWhite = create_lambertian_material((Color){1, 1, 1}, 0.5);
	Material glass = create_dielectric_material((Color){0, 0, 0}, 0.8, 0);

	add_sphere(scene, 0, -101, -8, 100, lambertianWhite);
	add_sphere(scene, 1.5, 1, -8, 1, lightSource);
	add_sphere(scene, 1.5, 0, -4, 0.5, lightSourceGreen);
	add_sphere(scene, -1.5, 0, -6, 1, glass);

	return scene;
}

Scene *load_scene2() {
	Scene *scene = init_scene((Color){0.5, 0.7, 1});

	Material lightSource = create_light_source_material((Color){1, 1, 1});
	Material lightSourceGreen = create_light_source_material((Color){0, 1, 0});
	Material lambertianRed = create_lambertian_material((Color){1, 0, 0}, 0.5);
	Material lambertianWhite = create_lambertian_material((Color){1, 1, 1}, 0.5);
	Material mirror = create_metal_material((Color){0, 0, 0}, 1, 0);
	Material glass = create_dielectric_material((Color){0, 0, 0}, 0.8, 0);

	add_sphere(scene, -10, 10, 0, 5, lightSource);
	add_sphere(scene, 0, -101, -8, 100, lambertianWhite);
	add_sphere(scene, -2, 0, -5, 1, lambertianRed);
	add_sphere(scene, 0, 0, -10, 1, lightSourceGreen);
	add_sphere(scene, 2, 0, -8, 1, mirror);
	add_sphere(scene, 3, 0, -5, 1, glass);

	return scene;
}

Scene *load_scene3() {
	Scene *scene = init_scene((Color){0, 0, 0});

	Material lightSource = create_light_source_material((Color){50, 50, 25});
	Material white = create_lambertian_material((Color){1, 1, 1}, 0.5);
	Material red = create_lambertian_material((Color){1, 0, 0}, 0.5);
	Material green = create_lambertian_material((Color){0, 1, 0}, 0.5);
	Material mirror = create_metal_material((Color){0, 0, 0}, 1, 0);
	Material glass = create_dielectric_material((Color){0, 0, 0}, 0.95, 0);

	add_sphere(scene, 0, 54.98, -10, 50, lightSource);
	add_sphere(scene, 0, 105, -10, 100, white);
	add_sphere(scene, 105, 0, -5, 100, green);
	add_sphere(scene, -105, 0, -5, 100, red);
	add_sphere(scene, 0, 0, -120, 100, white);
	add_sphere(scene, 0, -105, -10, 100, white);
	add_sphere(scene, -2.5, -3, -10, 2, glass);
	add_sphere(scene, 2, -4, -15, 1, mirror);

	return scene;
}
