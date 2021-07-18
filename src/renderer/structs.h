#ifndef RENDERER_STRUCTS_H
#define RENDERER_STRUCTS_H

#include <CL/cl.h>

typedef struct CLProgram {
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	cl_mem imageBuff;
	cl_mem voxelBuff;
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

typedef struct Voxel {
	cl_int3 pos;
	Material material;
} Voxel;

typedef struct Scene {
	cl_int voxelCount;
	Voxel *voxels;
	cl_float3 bgColor;
	cl_float bgBrightness;
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

#endif