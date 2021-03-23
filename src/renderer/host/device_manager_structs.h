#ifndef DEVICE_MANAGER_STRUCTS_H
#define DEVICE_MANAGER_STRUCTS_H

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

#endif