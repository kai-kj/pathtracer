#include <time.h>
#include "k_util.h"
#include "renderer.h"

#define FILE_NAME "data/kernel/pathtracer.cl"
#define KERNEL_NAME "renderer"
#define PROGRAM_ARGS "-Werror -cl-mad-enable -cl-no-signed-zeros -cl-fast-relaxed-math"

//---- private  --------------------------------------------------------------//

static void print_source_read_error() {
	msg("Failed to read source file\n");
}

static void print_program_build_error(cl_device_id device, cl_program program) {
	msg("Failed to build program\n");

	char buffer[0x100000];
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
	msg("%s\n", buffer);
}

//---- public ----------------------------------------------------------------//

Renderer *create_renderer() {
	srand(time(NULL));

	Renderer *renderer = malloc(sizeof(Renderer));

	renderer->scene = (Scene){
		0,
		NULL,
		{.x = 0, .y = 0, .z = 0}
	};

	// TODO: check platform count
	cl_uint platformNum;
	clGetPlatformIDs(1, &renderer->clProgram.platform, &platformNum);
	cl_uint deviceNum;
	clGetDeviceIDs(renderer->clProgram.platform, CL_DEVICE_TYPE_GPU, 2, &renderer->clProgram.device, &deviceNum);

	renderer->clProgram.context = clCreateContext(0, 1, &renderer->clProgram.device, NULL, NULL, NULL);
	renderer->clProgram.queue = clCreateCommandQueueWithProperties(renderer->clProgram.context, renderer->clProgram.device, 0, NULL);

	char *source = read_file(FILE_NAME);

	if(source == NULL) {
		print_source_read_error();
		exit(1);
	}

	renderer->clProgram.program = clCreateProgramWithSource(renderer->clProgram.context, 1, (const char**)&source, NULL, NULL);

	free(source);

	if(clBuildProgram(renderer->clProgram.program, 0, NULL, PROGRAM_ARGS, NULL, NULL) != CL_SUCCESS) {
		print_program_build_error(renderer->clProgram.device, renderer->clProgram.program);
		exit(1);
	}

	renderer->clProgram.kernel = clCreateKernel(renderer->clProgram.program, KERNEL_NAME, NULL);

	return renderer;
}

void destroy_renderer(Renderer *renderer) {
	clReleaseMemObject(renderer->clProgram.imageBuff);
	clReleaseProgram(renderer->clProgram.program);
	clReleaseKernel(renderer->clProgram.kernel);
	clReleaseCommandQueue(renderer->clProgram.queue);
	clReleaseContext(renderer->clProgram.context);
}
