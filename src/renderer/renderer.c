#include <time.h>
#include <k_tools/k_util.h>
#include "renderer.h"

#define FILE_NAME "data/kernel/pathtracer.cl"
#define KERNEL_NAME "renderer"
#define PROGRAM_ARGS "-Werror -cl-mad-enable -cl-no-signed-zeros -cl-fast-relaxed-math"

//---- private  --------------------------------------------------------------//

static void _print_source_read_error() {
	msg("Failed to read source file\n");
}

static void _print_program_build_error(cl_device_id device, cl_program program) {
	msg("Failed to build program\n");

	char buffer[0x100000];
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
	msg("%s\n", buffer);
}

//---- public ----------------------------------------------------------------//

RendererStatus create_renderer() {
	srand(time(NULL));

	r.scene.voxelCount = 0;
	r.clProgram.voxelBuff = NULL;
	r.clProgram.imageBuff = NULL;
	r.scene.voxels = NULL;

	// TODO: check platform count
	cl_uint platformNum;
	clGetPlatformIDs(1, &r.clProgram.platform, &platformNum);
	cl_uint deviceNum;
	clGetDeviceIDs(r.clProgram.platform, CL_DEVICE_TYPE_GPU, 2, &r.clProgram.device, &deviceNum);

	r.clProgram.context = clCreateContext(0, 1, &r.clProgram.device, NULL, NULL, NULL);
	r.clProgram.queue = clCreateCommandQueueWithProperties(r.clProgram.context, r.clProgram.device, 0, NULL);

	char *source = read_file(FILE_NAME);

	if(source == NULL) {
		_print_source_read_error();
		return RENDERER_FAULURE;
	}

	r.clProgram.program = clCreateProgramWithSource(r.clProgram.context, 1, (const char**)&source, NULL, NULL);

	free(source);

	if(clBuildProgram(r.clProgram.program, 0, NULL, PROGRAM_ARGS, NULL, NULL) != CL_SUCCESS) {
		_print_program_build_error(r.clProgram.device, r.clProgram.program);
		return RENDERER_FAULURE;
	}

	r.clProgram.kernel = clCreateKernel(r.clProgram.program, KERNEL_NAME, NULL);

	return RENDERER_SUCCESS;
}

RendererStatus destroy_renderer() {
	clReleaseMemObject(r.clProgram.imageBuff);

	if(r.scene.voxels != NULL) {
		free(r.scene.voxels);
		r.scene.voxels = NULL;
	}

	if(r.clImage.data != NULL) {
		free(r.clImage.data);
		r.clImage.data = NULL;
	}

	if(r.clProgram.voxelBuff != NULL) {
		clReleaseMemObject(r.clProgram.voxelBuff);
		r.clProgram.voxelBuff = NULL;
	}

	if(r.clProgram.imageBuff != NULL) {
		clReleaseMemObject(r.clProgram.imageBuff);
		r.clProgram.imageBuff = NULL;
	}

	clReleaseProgram(r.clProgram.program);
	clReleaseKernel(r.clProgram.kernel);
	clReleaseCommandQueue(r.clProgram.queue);
	clReleaseContext(r.clProgram.context);

	return RENDERER_SUCCESS;
}
