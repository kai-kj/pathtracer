#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#include "../../include/k_util.h"

typedef struct CLProgram {
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;

} CLProgram;

CLProgram create_cl_program(char *fileName, char *kernelName);
int run_cl_program(CLProgram clProgram, size_t workItems, int verbose);
void destroy_cl_program(CLProgram info);