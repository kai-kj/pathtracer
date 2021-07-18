#ifndef RENDERER_H
#define RENDERER_H

#include <math.h>
#include <time.h>
#include <k_tools/k_image.h>
#include <k_tools/k_util.h>
#include "structs.h"

typedef char RendererStatus;
#define RENDERER_SUCCESS 0
#define RENDERER_FAILURE -1

extern Renderer r;

//---- renderer --------------------------------------------------------------//

RendererStatus create_renderer();
RendererStatus destroy_renderer();

//---- image -----------------------------------------------------------------//

RendererStatus set_image_properties(int width, int height);
RendererStatus begin_image_rendering();
RendererStatus render_sample(int sampleNumber);
RendererStatus read_image();
RendererStatus end_image_rendering();
RendererStatus render_image_to_file(int samples, char *fileName);

//---- scene -----------------------------------------------------------------//

RendererStatus set_background_properties(float red, float green, float blue, float brightness);
RendererStatus create_scene();
RendererStatus destroy_scene();
RendererStatus add_voxel(int x, int y, int z, Material material);

//---- camera ----------------------------------------------------------------//

RendererStatus set_camera_properties(
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
