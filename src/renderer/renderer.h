#ifndef RENDERER_H
#define RENDERER_H

#include <CL/cl.h>

//---- structs----------------------------------------------------------------//

typedef struct CLProgram {
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	cl_mem imageBuff;
	cl_mem boxBuff;
} CLProgram;

typedef struct CLImage {
	cl_int2 size;
	cl_float3 *data;
} CLImage;

typedef struct Material {
	cl_uchar type;
	cl_float3 color;

	union {
		struct {
			cl_float brightness;
		} lightSource;

		struct {
		} lambertian;

		struct {
			cl_float tint;
			cl_float fuzz;
		} metal;

		struct {
			cl_float tint;
			cl_float fuzz;
			cl_float refIdx;
		} dielectric;
	} details;
} Material;

typedef struct AABB {
	cl_float3 lb;
	cl_float3 rt;
	Material material;
} AABB;

typedef struct Scene {
	cl_int boxCount;
	AABB *boxes;
	cl_float3 bgColor;
} Scene;

typedef struct Camera {
	cl_float3 pos;
	cl_float3 rot;
	cl_float sensorWidth;
	cl_float focalLength;
	cl_float aperture;
	cl_float exposure;
} Camera;

typedef struct Renderer {
	CLProgram clProgram;
	CLImage clImage;
	Scene scene;
	Camera camera;
} Renderer;

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
