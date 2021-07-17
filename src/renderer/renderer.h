#ifndef RENDERER_H
#define RENDERER_H

#include "k_image.h"
#include "renderer_structs.h"

//---- renderer --------------------------------------------------------------//

Renderer *create_renderer();
void destroy_renderer(Renderer *renderer);

//---- image -----------------------------------------------------------------//

void set_image_properties(Renderer *renderer, int width, int height);
k_Image *render_image(Renderer *renderer, int samples, int verbose);
void render_image_to_file(Renderer *renderer, int samples, char *fileName, int verbose);

//---- scene -----------------------------------------------------------------//

void set_background_color(Renderer *r, float red, float green, float blue);
void create_scene(Renderer *r, int width, int height, int depth);
void destroy_scene(Renderer *r);
void add_voxel(Renderer *r, int x, int y, int z, Material material);

//---- camera ----------------------------------------------------------------//

void set_camera_properties(
	Renderer *renderer,
	float x, float y, float z,
	float rotX, float rotY, float rotZ,
	float sensorWidth, float focalLength, float aperture, float exposure
);

//---- material --------------------------------------------------------------//

Material create_light_source_material(float r, float g, float b, float brightness);
Material create_lambertian_material(float r, float g, float b);
Material create_metal_material(float r, float g, float b, float tint, float fuzz);
Material create_dielectric_material(float r, float g, float b, float tint, float fuzz, float refIdx);

#endif
