#ifndef RENDERER_STRUCT_H
#define RENDERER_STRUCT_H

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
	cl_mem materialBuff;
} CLProgram;

typedef struct CLImage {
	cl_int2 size;
	cl_float3 *data;
} CLImage;

typedef unsigned int MaterialID;

// TODO: union?
typedef struct Material {
	cl_int type;
	cl_float3 color;
	cl_float tint;
	cl_float fuzzyness;
	cl_float refIdx;
} Material;

typedef struct Scene {
	cl_int3 size;
	cl_int *voxels;
	cl_int materialCount;
	Material *materials;
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

#endif