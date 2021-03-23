#ifndef MATERIAL_STRUCTS_H
#define MATERIAL_STRUCTS_H

#include <CL/cl.h>

typedef struct Material {
	cl_int type;
	cl_float3 color;
	cl_float tint;
	cl_float fuzzyness;
	cl_float refIdx;
} Material;

#endif