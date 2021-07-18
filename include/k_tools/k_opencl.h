#ifndef K_OPENCL_H
#define K_OPENCL_H

//---- definition ------------------------------------------------------------//

#include <CL/cl.h>

#define K_CL_SUCCES 0
#define K_CL_FILE_READ_ERROR 1
#define K_CL_PROGRAM_BUILD_ERROR 2

typedef struct k_CLProgram {
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_program program;
	cl_kernel kernel;
	int status;
} k_CLProgram;

k_CLProgram k_create_cl_program(const char *fileName, const char *kernelName, const char *args);
void k_print_error_message(k_CLProgram clProgram);

//---- implementation --------------------------------------------------------//

// TODO: uncomment
#ifdef K_OPENCL_IMPLEMENTATION

#include <stdlib.h>
#include <k_tools/k_util.h>

static void _print_program_build_error(cl_device_id device, cl_program program) {
	char buffer[0x100000];
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
	msg("%s\n", buffer);
}

void k_print_error_message(k_CLProgram clProgram) {
	switch(clProgram.status) {
		case K_CL_SUCCES:
			msg("No errors found\n");
			break;
		
		case K_CL_FILE_READ_ERROR:
			msg("Failed to read file\n");
			break;
		
		case K_CL_PROGRAM_BUILD_ERROR:
			msg("Failed to build program\n");
			_print_program_build_error(clProgram.device, clProgram.program);
			break;
	}
}

k_CLProgram k_create_cl_program(const char *fileName, const char *kernelName, const char *args) {
	k_CLProgram clProgram;
	cl_platform_id platform;

	// TODO: check platform and device numbers
	clGetPlatformIDs(1, &platform, NULL);
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &clProgram.device, NULL);

	clProgram.context = clCreateContext(0, 1, &clProgram.device, NULL, NULL, NULL);
	clProgram.queue = clCreateCommandQueueWithProperties(clProgram.context, clProgram.device, 0, NULL);

	char *source = read_file((char *)fileName);

	if(source == NULL) {
		clProgram.status = K_CL_FILE_READ_ERROR;
		return clProgram;
	}

	clProgram.program = clCreateProgramWithSource(clProgram.context, 1, (const char**)&source, NULL, NULL);

	free(source);

	if(clBuildProgram(clProgram.program, 0, NULL, args, NULL, NULL) != CL_SUCCESS) {
		clProgram.status = K_CL_PROGRAM_BUILD_ERROR;
		return clProgram;
	}

	clProgram.kernel = clCreateKernel(clProgram.program, kernelName, NULL);

	clProgram.status = K_CL_SUCCES;
	return clProgram;
}

void k_create_buffer(k_CLProgram *CLProgram) {
	
}

#endif

#endif