
function main()
	WIDTH = 400
	HEIGHT = 300
	SAMPLES = 1000
	MAX_DEPTH = 50
	WALL_RADIUS = 10000

	r = renderer.create_renderer();

	renderer.set_image_properties(r, WIDTH, HEIGHT, MAX_DEPTH);

	renderer.set_background_color(r, 0, 0, 0);

	lightSource = renderer.create_light_source_material(5, 5, 2.5);
	white = renderer.create_lambertian_material(1, 1, 1);
	red = renderer.create_lambertian_material(1, 0, 0);
	green = renderer.create_lambertian_material(0, 1, 0);
	mirror = renderer.create_metal_material(0, 0, 0, 0, 0);
	greenGlass = renderer.create_dielectric_material(0, 1, 0, 0.8, 0, 0.95);

	renderer.add_sphere(r, 0, 54.98, -10, 50, lightSource);

	renderer.add_sphere(r, 0, WALL_RADIUS + 5, -10, WALL_RADIUS, white);
	renderer.add_sphere(r, 0, -(WALL_RADIUS + 5), -10, WALL_RADIUS, white);
	renderer.add_sphere(r, 0, 0, -(WALL_RADIUS + 20), WALL_RADIUS, white);
	-- add_sphere(r, 0, 0, WALL_RADIUS + 10, WALL_RADIUS, mirror);

	renderer.add_sphere(r, WALL_RADIUS + 5, 0, -5, WALL_RADIUS, green);
	renderer.add_sphere(r, -(WALL_RADIUS + 5), 0, -5, WALL_RADIUS, red);
	
	renderer.add_sphere(r, -2.5, -3, -10, 2, greenGlass);
	renderer.add_sphere(r, 2, -3.5, -15, 1.5, mirror);

	renderer.set_camera_properties(
		r,
		0, 0, 10,
		0, 0, 0,
		10, 20, 0.001, 1000
	);

	renderer.render_to_file(r, SAMPLES, "render.png", 1);

	renderer.destroy_renderer(r);
end