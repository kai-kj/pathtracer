#include <math.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../include/stb_image_write.h"

#include "../../include/k_util.h"

#include "renderer.h"

#define FILE_NAME "src/cl/main.cl"
#define KERNEL_NAME "main"
#define PROGRAM_ARGS "-I src/cl" // -cl-mad-enable -cl-no-signed-zeros -cl-fast-relaxed-math

//---- private functions -----------------------------------------------------//

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

static char *get_file_ext(char *filename) {
	char *dot = strrchr(filename, '.');

	if(!dot || dot == filename)
		return "";
	
	return dot + 1;
}

static void gamma_correct_CLImage(CLImage *image) {
	int pixelCount = image->size.x * image->size.y;

	for(int i = 0; i < pixelCount; i++) {
		image->data[i].x = sqrt(image->data[i].x);
		image->data[i].y = sqrt(image->data[i].y);
		image->data[i].z = sqrt(image->data[i].z);
	}
}

//---- public functions ------------------------------------------------------//

Renderer *create_renderer() {
	Renderer *renderer = malloc(sizeof(Renderer));

	renderer->image.data = NULL;
	renderer->scene.data = NULL;

	// get platforms
	cl_uint platformNum;
	clGetPlatformIDs(1, &renderer->program.platform, &platformNum);

	// get devices
	cl_uint deviceNum;
	clGetDeviceIDs(renderer->program.platform, CL_DEVICE_TYPE_GPU, 2, &renderer->program.device, &deviceNum);

	// create context and command queue
	renderer->program.context = clCreateContext(0, 1, &renderer->program.device, NULL, NULL, NULL);
	renderer->program.queue = clCreateCommandQueueWithProperties(renderer->program.context, renderer->program.device, 0, NULL);

	// load program
	const char *source = read_file(FILE_NAME);
	renderer->program.program = clCreateProgramWithSource(renderer->program.context, 1, &source, NULL, NULL);
	free((void *)source);

	// build program
	if(clBuildProgram(renderer->program.program, 0, NULL, PROGRAM_ARGS, NULL, NULL) != CL_SUCCESS) {
		msg("Failed to build program\n");

		char buffer[0x100000];
		clGetProgramBuildInfo(renderer->program.program, renderer->program.device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
		msg("%s\n", buffer);
		exit(1);
	}

	// create kernel object
	renderer->program.kernel = clCreateKernel(renderer->program.program, KERNEL_NAME, NULL);

	return renderer;
}

void set_image_properties(Renderer *renderer, int width, int height) {
	renderer->image.size = (cl_int2){.x = width, .y = height};
	renderer->image.data = malloc(sizeof(cl_float3) * width * height);
	renderer->program.imageBuff = clCreateBuffer(renderer->program.context, CL_MEM_READ_WRITE, sizeof(cl_float3) * width * height, NULL, NULL);
}

void set_background_color(Renderer *renderer, float r, float g, float b) {
	renderer->scene.bgColor = (cl_float3){.x = r, .y = g, .z = b};
}

Material create_lambertian_material(float r, float g, float b) {
	return (Material){1, (cl_float3){.x = r, .y = g, .z = b}, 0, 0, 0};
}

Material create_metal_material(float r, float g, float b,  float tint, float fuzzyness) {
	return (Material){2, (cl_float3){.x = r, .y = g, .z = b}, tint, fuzzyness, 0};
}

Material create_dielectric_material(float r, float g, float b, float tint, float fuzzyness, float refIdx) {
	return (Material){3, (cl_float3){.x = r, .y = g, .z = b}, tint, fuzzyness, refIdx};
}

Material create_light_source_material(float r, float g, float b) {
	return (Material){0, (cl_float3){.x = r, .y = g, .z = b}, 0, 0, 0};
}

void set_camera_properties(Renderer *renderer, float x, float y, float z, float rotX, float rotY, float rotZ, float sensorWidth, float focalLength, float aperture, float exposure) {
	renderer->camera = (Camera){.sensorWidth = sensorWidth, .focalLength = focalLength, .aperture = aperture, .exposure = exposure};
	renderer->camera.pos = (cl_float3){.x = x, .y = y, .z = z};
	renderer->camera.rot = (cl_float3){.x = rotX, .y = rotY, .z = rotZ};
}

Image *render(Renderer *renderer, int samples, int verbose) {
	// create buffer for voxels
	int voxelCount = renderer->scene.size.x * renderer->scene.size.y * renderer->scene.size.z;
	renderer->program.sceneBuff = clCreateBuffer(renderer->program.context, CL_MEM_READ_ONLY, sizeof(cl_int) * voxelCount, NULL, NULL);
	clEnqueueWriteBuffer(renderer->program.queue, renderer->program.sceneBuff, CL_TRUE, 0, sizeof(cl_int) * voxelCount, renderer->scene.data, 0, NULL, NULL);

	// set kernel arguments
	clSetKernelArg(renderer->program.kernel, 0, sizeof(cl_mem), &renderer->program.imageBuff);
	clSetKernelArg(renderer->program.kernel, 1, sizeof(cl_int2), &renderer->image.size);
	clSetKernelArg(renderer->program.kernel, 2, sizeof(cl_mem), &renderer->program.sceneBuff);
	clSetKernelArg(renderer->program.kernel, 3, sizeof(cl_int3), &renderer->scene.size);
	clSetKernelArg(renderer->program.kernel, 4, sizeof(cl_float3), &renderer->scene.bgColor);
	clSetKernelArg(renderer->program.kernel, 5, sizeof(Camera), &renderer->camera);

	// render image
	size_t pixelCount = renderer->image.size.x * renderer->image.size.y;

	cl_int ret = clEnqueueNDRangeKernel(renderer->program.queue, renderer->program.kernel, 1, NULL, &pixelCount, NULL, 0, NULL, NULL);

	if(ret != CL_SUCCESS) {
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

		destroy_renderer(renderer);
		
		exit(1);
	}
	
	clFinish(renderer->program.queue);

	// read image
	clEnqueueReadBuffer(renderer->program.queue, renderer->program.imageBuff, CL_TRUE, 0, sizeof(cl_float3) * pixelCount, renderer->image.data, 0, NULL, NULL);
	
	clFinish(renderer->program.queue);

	printf("(%f, %f, %f)\n", renderer->image.data[0].x, renderer->image.data[0].y, renderer->image.data[0].z);
	printf("(%f, %f, %f)\n", renderer->image.data[1].x, renderer->image.data[1].y, renderer->image.data[1].z);

	// create image
	gamma_correct_CLImage(&renderer->image);
	Image *image = CLImage_to_Image(renderer->image);

	// cleanup
	clReleaseMemObject(renderer->program.sceneBuff);

	return image;
}

void render_to_file(Renderer *renderer, int samples, char *fileName, int verbose) {
	Image *image = render(renderer, samples, verbose);

	write_image(image, fileName);
	
	destroy_image(image);
}

void destroy_renderer(Renderer *renderer) {
	clReleaseMemObject(renderer->program.imageBuff);
	clReleaseProgram(renderer->program.program);
	clReleaseKernel(renderer->program.kernel);
	clReleaseCommandQueue(renderer->program.queue);
	clReleaseContext(renderer->program.context);
}

void write_image(Image *image, char *fileName) {
	char *format = get_file_ext(fileName);

	if(strcmp(format, "png") == 0) {
		stbi_write_png(fileName, image->width, image->height, 3, image->data, 0);

	} else if(strcmp(format, "bmp") == 0) {
		stbi_write_bmp(fileName, image->width, image->height, 3, image->data);

	} else if(strcmp(format, "jpg") == 0 || strcmp(format, "jpeg") == 0) {
		stbi_write_jpg(fileName, image->width, image->height, 3, image->data, 95);

	} else if(strcmp(format, "tga") == 0) {
		stbi_write_tga(fileName, image->width, image->height, 3, image->data);

	}
}

void destroy_image(Image *image) {
	free(image->data);
	free(image);
}