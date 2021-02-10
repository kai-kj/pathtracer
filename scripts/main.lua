function main()
	WIDTH = 640
	width = 360
	samples = 2000
	maxDepth = 50
	wallRadius = 10000

	length = 10
	fps = 30
	frames = length * fps

	r = renderer.create_renderer();

	renderer.set_image_properties(r, WIDTH, width, maxDepth);

	renderer.set_background_color(r, 0, 0, 0);

	lightSource = renderer.create_light_source_material(10, 10, 10);
	white = renderer.create_lambertian_material(1, 1, 1);
	red = renderer.create_lambertian_material(1, 0, 0);
	green = renderer.create_lambertian_material(0, 1, 0);
	mirror = renderer.create_metal_material(0, 0, 0, 0, 0);
	greenGlass = renderer.create_dielectric_material(0, 1, 0, 0.8, 0, 0.95);

	for i = 0, frames do
		print("frame "..i)

		renderer.clear_scene(r)

		-- renderer.add_sphere(r, 0, 54.98, -10, 50, lightSource);

		-- walls
		renderer.add_sphere(r, 0, wallRadius + 6, -10, wallRadius, white);
		renderer.add_sphere(r, 0, -(wallRadius + 6), -10, wallRadius, white);
		renderer.add_sphere(r, 0, 0, -(wallRadius + 20), wallRadius, white);
		renderer.add_sphere(r, 0, 0, (wallRadius + 20), wallRadius, white);
		renderer.add_sphere(r, wallRadius + 6, 0, -5, wallRadius, green);
		renderer.add_sphere(r, -(wallRadius + 6), 0, -5, wallRadius, red);
		
		-- center piece
		renderer.add_sphere(r, -1, 0, -10, 1, greenGlass);

		-- light
		radius = 2
		center = {x = -1, y = 0, z = -10}
		offset = {
			x = radius * math.cos(2 * math.pi * (i / frames)),
			y = math.sin(2 * math.pi * (i / frames)),
			z = radius * math.sin(2 * math.pi * (i / frames))
		}
		
		renderer.add_sphere(
			r,
			center.x + offset.x, center.y + offset.y, center.z + offset.z,
			0.5,
			lightSource
		);
		
		-- mirror
		radius = 3.5
		offset = {
			x = radius * math.cos(-4 * math.pi * (i / frames)),
			y = 0,
			z = radius * math.sin(-4 * math.pi * (i / frames))
		}

		renderer.add_sphere(
			r,
			center.x + offset.x, center.y + offset.y, center.z + offset.z,
			1,
			mirror
		);

		radius = 5
		startOffset = 3 * math.pi / 4

		offset = {
			x = 0,
			y = radius * math.cos(2 * math.pi * (i / frames) + startOffset),
			z = radius * math.sin(2 * math.pi * (i / frames) + startOffset)
		}

		direction = {
			x = 2 * math.pi * (i / frames) - math.pi / 2 + startOffset,
			y = 0,
			z = 0,
		}

		renderer.set_camera_properties(
			r,
			center.x + offset.x, center.y + offset.y, center.z + offset.z,
			direction.x, direction.y, direction.z,
			15, 20, 0.001, 1000
		);

		renderer.render_to_file(r, samples, "clip/frame"..i..".png", 0);

	end

	renderer.destroy_renderer(r);

	os.execute("ffmpeg -y -framerate "..fps.." -i clip/frame%d.png -c:v libx264 -profile:v high -crf 20 -pix_fmt yuv420p output.mp4")
end