#include <math.h>
#include "k_util.h"
#include "renderer.h"

//---- private ---------------------------------------------------------------//

static void _print_kernel_run_error(cl_int ret) {
	msg("Failed to run kernel: ");

	switch(ret) {
		case CL_INVALID_PROGRAM_EXECUTABLE: msg("INVALID_PROGRAM_EXECUTABLE\n"); break;
		case CL_INVALID_COMMAND_QUEUE: msg("INVALID_COMMAND_QUEUE\n"); break;
		case CL_INVALID_KERNEL: msg("INVALID_KERNEL\n"); break;
		case CL_INVALID_CONTEXT: msg("INVALID_CONTEXT\n"); break;
		case CL_INVALID_KERNEL_ARGS: msg("INVALID_KERNEL_ARGS\n"); break;
		case CL_INVALID_WORK_DIMENSION: msg("INVALID_WORK_DIMENSION\n"); break;
		case CL_INVALID_GLOBAL_WORK_SIZE: msg("INVALID_GLOBAL_WORK_SIZE\n"); break;
		case CL_INVALID_GLOBAL_OFFSET: msg("INVALID_GLOBAL_OFFSET\n"); break;
		case CL_INVALID_WORK_GROUP_SIZE: msg("INVALID_WORK_GROUP_SIZE\n"); break;
		case CL_MISALIGNED_SUB_BUFFER_OFFSET: msg("MISALIGNED_SUB_BUFFER_OFFSET\n"); break;
		case CL_INVALID_IMAGE_SIZE: msg("INVALID_IMAGE_SIZE\n"); break;
		case CL_IMAGE_FORMAT_NOT_SUPPORTED: msg("IMAGE_FORMAT_NOT_SUPPORTED\n"); break;
		case CL_OUT_OF_RESOURCES: msg("OUT_OF_RESOURCES\n"); break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE: msg("MEM_OBJECT_ALLOCATION_FAILURE\n"); break;
		case CL_INVALID_EVENT_WAIT_LIST: msg("INVALID_EVENT_WAIT_LIST\n"); break;
		case CL_INVALID_OPERATION: msg("INVALID_OPERATION\n"); break;
		case CL_OUT_OF_HOST_MEMORY: msg("OUT_OF_HOST_MEMORY\n"); break;
		default: msg("NO_ERROR_CODE\n"); break;
	}
}

static void _render_sample(Renderer *renderer, int sampleNumber, int verbose) {
	cl_ulong seed = rand();

	// non-constant arguments
	clSetKernelArg(renderer->clProgram.kernel, 6, sizeof(cl_int), &sampleNumber);
	clSetKernelArg(renderer->clProgram.kernel, 7, sizeof(cl_ulong), &seed);

	size_t pixelCount = renderer->clImage.size.x * renderer->clImage.size.y;

	cl_int ret = clEnqueueNDRangeKernel(renderer->clProgram.queue, renderer->clProgram.kernel, 1, NULL, &pixelCount, NULL, 0, NULL, NULL);

	if(ret != CL_SUCCESS) {
		_print_kernel_run_error(ret);
		destroy_renderer(renderer);
		exit(1);
	}

	clFinish(renderer->clProgram.queue);
}

static void _gamma_correct_CLImage(CLImage image) {
	int pixelCount = image.size.x * image.size.y;

	for(int i = 0; i < pixelCount; i++) {
		image.data[i].x = sqrt(image.data[i].x);
		image.data[i].y = sqrt(image.data[i].y);
		image.data[i].z = sqrt(image.data[i].z);
	}
}

static k_Image *_CLImage_to_Image(CLImage clImage) {
	k_Image *image = k_create_image(clImage.size.x, clImage.size.y);

	for(int i = 0; i < image->width * image->height; i++) {
		if(clImage.data[i].x < 0) clImage.data[i].x = 0;
		if(clImage.data[i].y < 0) clImage.data[i].y = 0;
		if(clImage.data[i].z < 0) clImage.data[i].z = 0;
		
		if(clImage.data[i].x > 1) clImage.data[i].x = 1;
		if(clImage.data[i].y > 1) clImage.data[i].y = 1;
		if(clImage.data[i].z > 1) clImage.data[i].z = 1;

		image->data[i * 3 + 0] = (int)(clImage.data[i].x * 255);
		image->data[i * 3 + 1] = (int)(clImage.data[i].y * 255);
		image->data[i * 3 + 2] = (int)(clImage.data[i].z * 255);
	}

	return image;
}

//---- public ----------------------------------------------------------------//

void set_image_properties(Renderer *renderer, int width, int height) {
	clReleaseMemObject(renderer->clProgram.imageBuff);

	renderer->clImage.size = (cl_int2){.x = width, .y = height};
	renderer->clImage.data = malloc(sizeof(cl_float3) * width * height);
	renderer->clProgram.imageBuff = clCreateBuffer(renderer->clProgram.context, CL_MEM_READ_WRITE, sizeof(cl_float3) * width * height, NULL, NULL);
}

k_Image *render_image(Renderer *renderer, int samples, int verbose) {
	renderer->clProgram.boxBuff = clCreateBuffer(renderer->clProgram.context, CL_MEM_READ_ONLY, sizeof(Voxel) * renderer->scene.voxelCount, NULL, NULL);
	clEnqueueWriteBuffer(renderer->clProgram.queue, renderer->clProgram.boxBuff, CL_TRUE, 0, sizeof(Voxel) * renderer->scene.voxelCount, renderer->scene.voxels, 0, NULL, NULL);

	// constant arguments
	clSetKernelArg(renderer->clProgram.kernel, 0, sizeof(cl_int2), &renderer->clImage.size);
	clSetKernelArg(renderer->clProgram.kernel, 1, sizeof(cl_mem), &renderer->clProgram.imageBuff);
	clSetKernelArg(renderer->clProgram.kernel, 2, sizeof(cl_int), &renderer->scene.voxelCount);
	clSetKernelArg(renderer->clProgram.kernel, 3, sizeof(cl_mem), &renderer->clProgram.boxBuff);
	clSetKernelArg(renderer->clProgram.kernel, 4, sizeof(cl_float3), &renderer->scene.bgColor);
	clSetKernelArg(renderer->clProgram.kernel, 5, sizeof(Camera), &renderer->camera);

	for(int i = 0; i < samples; i++) {
		float percent = (float)(i + 1) / (float)samples * 100.0f;
		msg("Rendering frame %d/%d (%0.1f%%)\n", i + 1, samples, percent);
		_render_sample(renderer, i, verbose);
	}
	
	clReleaseMemObject(renderer->clProgram.boxBuff);

	size_t pixelCount = renderer->clImage.size.x * renderer->clImage.size.y;

	clEnqueueReadBuffer(renderer->clProgram.queue, renderer->clProgram.imageBuff, CL_TRUE, 0, sizeof(cl_float3) * pixelCount, renderer->clImage.data, 0, NULL, NULL);

	clFinish(renderer->clProgram.queue);

	_gamma_correct_CLImage(renderer->clImage);

	k_Image *image = _CLImage_to_Image(renderer->clImage);

	return image;
}

void render_image_to_file(Renderer *renderer, int samples, char *fileName, int verbose) {
	k_Image *image = render_image(renderer, samples, verbose);

	k_write_image(image, fileName);
	
	k_destroy_image(image);
}