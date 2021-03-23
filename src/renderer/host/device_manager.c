#include <math.h>
#include <string.h>

#include "../../../include/k_util.h"

#include "device_manager.h"

#define FILE_NAME "src/renderer/device/renderer.cl"
#define KERNEL_NAME "main"
#define PROGRAM_ARGS "-I src/renderer/device" // -cl-mad-enable -cl-no-signed-zeros -cl-fast-relaxed-math

//---- private functions -----------------------------------------------------//

static void print_source_read_error() {
	msg("Failed to read source file\n");
}

static void print_program_build_error(cl_device_id device, cl_program program) {
	msg("Failed to build program\n");

	char buffer[0x100000];
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
	msg("%s\n", buffer);
}

static void print_kernel_run_error(cl_int ret) {
	msg("Failed to run kernel\n");

	switch(ret) {
		case CL_INVALID_PROGRAM_EXECUTABLE:
			msg("there is no successfully built program executable available for device associated with command_queue.\n");
			break;
		case CL_INVALID_COMMAND_QUEUE:
			msg("command_queue is not a valid host command-queue.\n");
			break;
		case CL_INVALID_KERNEL:
			msg("kernel is not a valid kernel object.\n");
			break;
		case CL_INVALID_CONTEXT:
			msg("context associated with command_queue and kernel are not the same or if the context associated with command_queue and events in event_wait_list are not the same.\n");
			break;
		case CL_INVALID_KERNEL_ARGS:
			msg("the kernel argument values have not been specified or if a kernel argument declared to be a pointer to a type does not point to a named address space.\n");
			break;
		case CL_INVALID_WORK_DIMENSION:
			msg("work_dim is not a valid value (i.e. a value between 1 and CL_​DEVICE_​MAX_​WORK_​ITEM_​DIMENSIONS).\n");
			break;
		case CL_INVALID_GLOBAL_WORK_SIZE:
			msg("any of the values specified in global_work_size[0], …​ global_work_size[work_dim - 1] exceed the maximum value representable by size_t on the device on which the kernel-instance will be enqueued.\n");
			msg("any of the values specified in global_work_size[0], …​ global_work_size[work_dim - 1] exceed the maximum value representable by size_t on the device on which the kernel-instance will be enqueued.\n");
			break;
		case CL_INVALID_GLOBAL_OFFSET:
			msg("the value specified in global_work_size + the corresponding values in global_work_offset for any dimensions is greater than the maximum value representable by size t on the device on which the kernel-instance will be enqueued.\n");
			break;
		case CL_INVALID_WORK_GROUP_SIZE:
			msg("local_work_size is specified and does not match the required work-group size for kernel in the program source.\n");
			msg("local_work_size is specified and is not consistent with the required number of sub-groups for kernel in the program source.\n");
			msg("local_work_size is specified and the total number of work-items in the work-group computed as local_work_size[0] × …​ local_work_size[work_dim - 1] is greater than the value specified by CL_​KERNEL_​WORK_​GROUP_​SIZE in the Kernel Object Device Queries table.\n");
			msg("the program was compiled with cl-uniform-work-group-size and the number of work-items specified by global_work_size is not evenly divisible by size of work-group given by local_work_size or by the required work-group size specified in the kernel source.\n");
			msg("the number of work-items specified in any of local_work_size[0], …​ local_work_size[work_dim - 1] is greater than the corresponding values specified by CL_​DEVICE_​MAX_​WORK_​ITEM_​SIZES[0], …​, CL_​DEVICE_​MAX_​WORK_​ITEM_​SIZES[work_dim - 1].\n");
			break;
		case CL_MISALIGNED_SUB_BUFFER_OFFSET:
			msg("a sub-buffer object is specified as the value for an argument that is a buffer object and the offset specified when the sub-buffer object is created is not aligned to CL_​DEVICE_​MEM_​BASE_​ADDR_​ALIGN value for device associated with queue. This error code is missing before version 1.1.\n");
			break;
		case CL_INVALID_IMAGE_SIZE:
			msg("an image object is specified as an argument value and the image dimensions (image width, height, specified or compute row and/or slice pitch) are not supported by device associated with queue.\n");
			break;
		case CL_IMAGE_FORMAT_NOT_SUPPORTED:
			msg("an image object is specified as an argument value and the image format (image channel order and data type) is not supported by device associated with queue.\n");
			break;
		case CL_OUT_OF_RESOURCES:
			msg("there is a failure to queue the execution instance of kernel on the command-queue because of insufficient resources needed to execute the kernel. For example, the explicitly specified local_work_size causes a failure to execute the kernel because of insufficient resources such as registers or local memory. Another example would be the number of read-only image args used in kernel exceed the CL_​DEVICE_​MAX_​READ_​IMAGE_​ARGS value for device or the number of write-only and read-write image args used in kernel exceed the CL_​DEVICE_​MAX_​READ_​WRITE_​IMAGE_​ARGS value for device or the number of samplers used in kernel exceed CL_​DEVICE_​MAX_​SAMPLERS for device.\n");
			break;
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:
			msg("there is a failure to allocate memory for data store associated with image or buffer objects specified as arguments to kernel.\n");
			break;
		case CL_INVALID_EVENT_WAIT_LIST:
			msg("event_wait_list is NULL and num_events_in_wait_list > 0, or event_wait_list is not NULL and num_events_in_wait_list is 0, or if event objects in event_wait_list are not valid events.\n");
			break;
		case CL_INVALID_OPERATION:
			msg("SVM pointers are passed as arguments to a kernel and the device does not support SVM or if system pointers are passed as arguments to a kernel and/or stored inside SVM allocations passed as kernel arguments and the device does not support fine grain system SVM allocations.\n");
			break;
		case CL_OUT_OF_HOST_MEMORY:
			msg("there is a failure to allocate resources required by the OpenCL implementation on the host.\n");
			break;
		default:
			break;
	}
}

void gamma_correct_CLImage(CLImage image) {
	int pixelCount = image.size.x * image.size.y;

	for(int i = 0; i < pixelCount; i++) {
		image.data[i].x = sqrt(image.data[i].x);
		image.data[i].y = sqrt(image.data[i].y);
		image.data[i].z = sqrt(image.data[i].z);
	}
}

static Image *CLImage_to_Image(CLImage clImage) {
	Image *image = malloc(sizeof(Image));
	image->width = clImage.size.x;
	image->height = clImage.size.y;

	image->data = malloc(sizeof(unsigned char) * 3 * image->width * image->height);

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

//---- public functions ------------------------------------------------------//

void create_kernel(Renderer *renderer) {
	// get platforms
	cl_uint platformNum;
	clGetPlatformIDs(1, &renderer->clProgram.platform, &platformNum);

	// get devices
	cl_uint deviceNum;
	clGetDeviceIDs(renderer->clProgram.platform, CL_DEVICE_TYPE_GPU, 2, &renderer->clProgram.device, &deviceNum);

	// create context and command queue
	renderer->clProgram.context = clCreateContext(0, 1, &renderer->clProgram.device, NULL, NULL, NULL);
	renderer->clProgram.queue = clCreateCommandQueueWithProperties(renderer->clProgram.context, renderer->clProgram.device, 0, NULL);

	// load program
	const char *source = read_file(FILE_NAME);

	if(source == NULL) {
		print_source_read_error();
		exit(1);
	}

	renderer->clProgram.program = clCreateProgramWithSource(renderer->clProgram.context, 1, &source, NULL, NULL);
	free((void *)source);

	// build program
	if(clBuildProgram(renderer->clProgram.program, 0, NULL, PROGRAM_ARGS, NULL, NULL) != CL_SUCCESS) {
		print_program_build_error(renderer->clProgram.device, renderer->clProgram.program);
		exit(1);
	}

	// create kernel object
	renderer->clProgram.kernel = clCreateKernel(renderer->clProgram.program, KERNEL_NAME, NULL);
}

void setup_kernel_args(Renderer *renderer) {
	// create buffer for voxels
	int voxelCount = renderer->scene.size.x * renderer->scene.size.y * renderer->scene.size.z;
	renderer->clProgram.voxelBuff = clCreateBuffer(renderer->clProgram.context, CL_MEM_READ_ONLY, sizeof(cl_int) * voxelCount, NULL, NULL);
	clEnqueueWriteBuffer(renderer->clProgram.queue, renderer->clProgram.voxelBuff, CL_TRUE, 0, sizeof(cl_int) * voxelCount, renderer->scene.voxels, 0, NULL, NULL);

	// create buffer for materials
	renderer->clProgram.materialBuff =  clCreateBuffer(renderer->clProgram.context, CL_MEM_READ_ONLY, sizeof(Material) * renderer->scene.materialCount, NULL, NULL);
	clEnqueueWriteBuffer(renderer->clProgram.queue, renderer->clProgram.materialBuff, CL_TRUE, 0, sizeof(Material) * renderer->scene.materialCount, renderer->scene.materials, 0, NULL, NULL);

	// set kernel arguments
	clSetKernelArg(renderer->clProgram.kernel, 0, sizeof(cl_mem), &renderer->clProgram.imageBuff);
	clSetKernelArg(renderer->clProgram.kernel, 1, sizeof(cl_int2), &renderer->image.size);
	clSetKernelArg(renderer->clProgram.kernel, 2, sizeof(cl_mem), &renderer->clProgram.voxelBuff);
	clSetKernelArg(renderer->clProgram.kernel, 3, sizeof(cl_int3), &renderer->scene.size);
	clSetKernelArg(renderer->clProgram.kernel, 4, sizeof(cl_mem), &renderer->clProgram.materialBuff);
	clSetKernelArg(renderer->clProgram.kernel, 5, sizeof(cl_int), &renderer->scene.materialCount);
	clSetKernelArg(renderer->clProgram.kernel, 6, sizeof(cl_float3), &renderer->scene.bgColor);
	clSetKernelArg(renderer->clProgram.kernel, 7, sizeof(Camera), &renderer->camera);
}

void run_kernel(Renderer *renderer, int samples) {
	size_t pixelCount = renderer->image.size.x * renderer->image.size.y;

	cl_int ret = clEnqueueNDRangeKernel(renderer->clProgram.queue, renderer->clProgram.kernel, 1, NULL, &pixelCount, NULL, 0, NULL, NULL);

	if(ret != CL_SUCCESS) {
		print_kernel_run_error(ret);
		release_kernel(renderer);
		exit(1);
	}

	clFinish(renderer->clProgram.queue);

	// read image
	clEnqueueReadBuffer(renderer->clProgram.queue, renderer->clProgram.imageBuff, CL_TRUE, 0, sizeof(cl_float3) * pixelCount, renderer->image.data, 0, NULL, NULL);
	clFinish(renderer->clProgram.queue);
}

Image *get_image(Renderer *renderer) {
	gamma_correct_CLImage(renderer->image);

	// create image
	Image *image = CLImage_to_Image(renderer->image);

	return image;
}

void release_kernel(Renderer *renderer) {
	clReleaseMemObject(renderer->clProgram.imageBuff);
	clReleaseMemObject(renderer->clProgram.voxelBuff);
	clReleaseProgram(renderer->clProgram.program);
	clReleaseKernel(renderer->clProgram.kernel);
	clReleaseCommandQueue(renderer->clProgram.queue);
	clReleaseContext(renderer->clProgram.context);
}