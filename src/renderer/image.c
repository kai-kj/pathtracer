#include <math.h>
#include <k_tools/k_util.h>
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

static void _gamma_correct_CLImage(CLImage image) {
	int pixelCount = image.size.x * image.size.y;

	for(int i = 0; i < pixelCount; i++) {
		image.data[i].x = sqrt(image.data[i].x);
		image.data[i].y = sqrt(image.data[i].y);
		image.data[i].z = sqrt(image.data[i].z);
	}
}

static k_Image *_CLImage_to_k_Image(CLImage clImage) {
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

RendererStatus set_image_properties(int width, int height) {
	if(r.clProgram.imageBuff != NULL) {
		clReleaseMemObject(r.clProgram.imageBuff);
		r.clProgram.imageBuff = NULL;
	}

	r.clImage.size = (cl_int2){.x = width, .y = height};
	r.clImage.data = malloc(sizeof(cl_float3) * width * height);
	r.clProgram.imageBuff = clCreateBuffer(r.clProgram.context, CL_MEM_READ_WRITE, sizeof(cl_float3) * width * height, NULL, NULL);

	return RENDERER_SUCCESS;
}

RendererStatus begin_image_rendering() {
	r.clProgram.voxelBuff = clCreateBuffer(r.clProgram.context, CL_MEM_READ_ONLY, sizeof(Voxel) * r.scene.voxelCount, NULL, NULL);
	clEnqueueWriteBuffer(r.clProgram.queue, r.clProgram.voxelBuff, CL_TRUE, 0, sizeof(Voxel) * r.scene.voxelCount, r.scene.voxels, 0, NULL, NULL);

	// constant arguments
	clSetKernelArg(r.clProgram.kernel, 0, sizeof(cl_int2), &r.clImage.size);
	clSetKernelArg(r.clProgram.kernel, 1, sizeof(cl_mem), &r.clProgram.imageBuff);
	clSetKernelArg(r.clProgram.kernel, 2, sizeof(cl_int), &r.scene.voxelCount);
	clSetKernelArg(r.clProgram.kernel, 3, sizeof(cl_mem), &r.clProgram.voxelBuff);
	clSetKernelArg(r.clProgram.kernel, 4, sizeof(cl_float3), &r.scene.bgColor);
	clSetKernelArg(r.clProgram.kernel, 5, sizeof(cl_float3), &r.scene.bgBrightness);
	clSetKernelArg(r.clProgram.kernel, 6, sizeof(Camera), &r.camera);

	return RENDERER_SUCCESS;
}

RendererStatus render_sample(int sampleNumber) {
	cl_ulong seed = rand();

	// non-constant arguments
	clSetKernelArg(r.clProgram.kernel, 7, sizeof(cl_int), &sampleNumber);
	clSetKernelArg(r.clProgram.kernel, 8, sizeof(cl_ulong), &seed);

	size_t pixelCount = r.clImage.size.x * r.clImage.size.y;

	cl_int ret = clEnqueueNDRangeKernel(r.clProgram.queue, r.clProgram.kernel, 1, NULL, &pixelCount, NULL, 0, NULL, NULL);

	if(ret != CL_SUCCESS) {
		_print_kernel_run_error(ret);
		return RENDERER_FAULURE;
	}

	clFinish(r.clProgram.queue);

	return RENDERER_SUCCESS;
}

RendererStatus end_image_rendering() {
	if(r.clProgram.voxelBuff != NULL) {
		clReleaseMemObject(r.clProgram.voxelBuff);
		r.clProgram.voxelBuff = NULL;
	}

	size_t pixelCount = r.clImage.size.x * r.clImage.size.y;

	clEnqueueReadBuffer(r.clProgram.queue, r.clProgram.imageBuff, CL_TRUE, 0, sizeof(cl_float3) * pixelCount, r.clImage.data, 0, NULL, NULL);

	clFinish(r.clProgram.queue);

	// _gamma_correct_CLImage(r.clImage);

	return RENDERER_SUCCESS;
}

RendererStatus render_image_to_file(int samples, char *fileName) {
	begin_image_rendering();

	for(int i = 0; i < samples; i++) {
		msg("%d/%d\n", i, samples);
		if(render_sample(i) == RENDERER_FAULURE) return RENDERER_FAULURE;
	}

	end_image_rendering();

	k_Image *image = _CLImage_to_k_Image(r.clImage);

	k_write_image(image, fileName);
	
	k_destroy_image(image);

	return RENDERER_SUCCESS;
}