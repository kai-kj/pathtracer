#include <math.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../include/stb_image_write.h"

#include "../../include/k_util.h"

#include "renderer.h"

#define FILE_NAME "src/cl/main.cl"
#define KERNEL_NAME "main"
#define PROGRAM_ARGS "-I src/cl -Werror -cl-mad-enable -cl-no-signed-zeros -cl-fast-relaxed-math"

//---- private functions -----------------------------------------------------//

Image *convert_to_image(cl_float3 *data, int width, int height) {
	Image *image = malloc(sizeof(Image));
	image->width = width;
	image->height = height;

	image->data = malloc(sizeof(unsigned char) * 3 * width * height);

	for(int i = 0; i < width * height; i++) {
		if(data[i].x < 0) data[i].x = 0;
		if(data[i].y < 0) data[i].y = 0;
		if(data[i].z < 0) data[i].z = 0;
		
		if(data[i].x > 1) data[i].x = 1;
		if(data[i].y > 1) data[i].y = 1;
		if(data[i].z > 1) data[i].z = 1;

		image->data[i * 3 + 0] = (int)(data[i].x * 255);
		image->data[i * 3 + 1] = (int)(data[i].y * 255);
		image->data[i * 3 + 2] = (int)(data[i].z * 255);
	}

	return image;
}

static char *get_filename_ext(char *filename) {
	char *dot = strrchr(filename, '.');

	if(!dot || dot == filename)
		return "";
	
	return dot + 1;
}

void gamma_correct_data(cl_float3 *data, int pixelCount) {
	for(int i = 0; i < pixelCount; i++) {
		data[i].x = sqrt(data[i].x);
		data[i].y = sqrt(data[i].y);
		data[i].z = sqrt(data[i].z);
	}
}

//---- public functions ------------------------------------------------------//

Renderer *create_renderer() {
	Renderer *renderer = malloc(sizeof(Renderer));

	renderer->sceneInfo.sphereCount = 0;
	renderer->sphereList = NULL;

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
	renderer->clProgram.program = clCreateProgramWithSource(renderer->clProgram.context, 1, &source, NULL, NULL);
	free((char *)source);

	// build program
	if(clBuildProgram(renderer->clProgram.program, 0, NULL, PROGRAM_ARGS, NULL, NULL) != CL_SUCCESS) {
		msg("Failed to build program\n");

		char buffer[0x100000];
		clGetProgramBuildInfo(renderer->clProgram.program, renderer->clProgram.device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
		msg("%s\n", buffer);
		exit(1);
	}

	// create kernel object
	renderer->clProgram.kernel = clCreateKernel(renderer->clProgram.program, KERNEL_NAME, NULL);

	return renderer;
}

void set_image_properties(Renderer *renderer, int width, int height, int maxRayDepth) {
	renderer->imageInfo.size = (cl_int2){.x = width, .y = height};
	renderer->imageInfo.maxRayDepth = maxRayDepth;

	renderer->clProgram.imageBuff = clCreateBuffer(renderer->clProgram.context, CL_MEM_READ_WRITE, sizeof(cl_float3) * width * height, NULL, NULL);
}

void set_background_color(Renderer *renderer, float r, float g, float b) {
	renderer->sceneInfo.color = (cl_float3){.x = r, .y = g, .z = b};
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

void add_sphere(Renderer *renderer, float x, float y, float z, float radius, Material material) {
	cl_float3 center = {.x = x, .y = y, .z = z};

	Sphere sphere = (Sphere){center, radius, material};
	int sphereCount = renderer->sceneInfo.sphereCount + 1;

	renderer->sphereList = realloc(renderer->sphereList, sizeof(Sphere) * sphereCount);
	renderer->sphereList[sphereCount - 1] = sphere;

	renderer->sceneInfo.sphereCount = sphereCount;
}

void set_camera_properties(Renderer *renderer, float x, float y, float z, float rotX, float rotY, float rotZ, float sensorWidth, float focalLength, float aperture, float exposure) {
	renderer->camera = (Camera){.sensorWidth = sensorWidth, .focalLength = focalLength, .aperture = aperture, .exposure = exposure};
	renderer->camera.pos = (cl_float3){.x = x, .y = y, .z = z};
	renderer->camera.rot = (cl_float3){.x = rotX, .y = rotY, .z = rotZ};
}

Image *render(Renderer *renderer, int samples, int verbose) {
	// create buffer for spheres
	renderer->clProgram.sphereBuff = clCreateBuffer(renderer->clProgram.context, CL_MEM_READ_ONLY, sizeof(Sphere) * renderer->sceneInfo.sphereCount, NULL, NULL);
	clEnqueueWriteBuffer(renderer->clProgram.queue, renderer->clProgram.sphereBuff, CL_TRUE, 0, sizeof(Sphere) * renderer->sceneInfo.sphereCount, renderer->sphereList, 0, NULL, NULL);

	// set permanent kernel arguments
	clSetKernelArg(renderer->clProgram.kernel, 0, sizeof(cl_mem), &renderer->clProgram.imageBuff);
	clSetKernelArg(renderer->clProgram.kernel, 1, sizeof(ImageInfo), &renderer->imageInfo);
	clSetKernelArg(renderer->clProgram.kernel, 2, sizeof(SceneInfo), &renderer->sceneInfo);
	clSetKernelArg(renderer->clProgram.kernel, 3, sizeof(Camera), &renderer->camera);
	clSetKernelArg(renderer->clProgram.kernel, 4, sizeof(cl_mem), &renderer->clProgram.sphereBuff);

	// render image
	double renderStartTime = get_time();

	size_t workItems = renderer->imageInfo.size.x * renderer->imageInfo.size.y;

	for(cl_int frame = 0; frame < samples; frame++) {
		// seed
		cl_ulong seed = rand();

		// kernel arguments
		clSetKernelArg(renderer->clProgram.kernel, 5, sizeof(cl_int), &frame);
		clSetKernelArg(renderer->clProgram.kernel, 6, sizeof(cl_ulong), &seed);

		// run program
		cl_int ret = clEnqueueNDRangeKernel(renderer->clProgram.queue, renderer->clProgram.kernel, 1, NULL, &workItems, NULL, 0, NULL, NULL);
			
		if(verbose) {
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

		if(ret != CL_SUCCESS) {
			clReleaseMemObject(renderer->clProgram.sphereBuff);
			destroy_renderer(renderer);

			return NULL;
		}
		
		clFinish(renderer->clProgram.queue);

		// print time
		if(verbose && frame % 10 == 9) {
			float frameTime = (get_time() - renderStartTime) / (frame + 1);
			float et = (samples - (frame * 1)) * frameTime;
			int h = sec_to_h(et);
			int min = sec_to_min(et) - h * 60;
			int sec = et - h * 360 - min * 60;
			msg("%d/%d samples (%f%%), ET: %02d:%02d:%02d\r", frame + 1, samples, (float)(frame + 1) / samples * 100, h, min, sec);
		}
	}

	if(verbose)
		printf("\n");

	// read image
	int pixelCount = renderer->imageInfo.size.x * renderer->imageInfo.size.y;

	cl_float3 *imageData = malloc(sizeof(cl_float3) * pixelCount);
	clEnqueueReadBuffer(renderer->clProgram.queue, renderer->clProgram.imageBuff, CL_TRUE, 0, sizeof(cl_float3) * pixelCount, imageData, 0, NULL, NULL);
	
	clFinish(renderer->clProgram.queue);

	gamma_correct_data(imageData, pixelCount);

	// create image
	Image *image = convert_to_image(imageData, renderer->imageInfo.size.x, renderer->imageInfo.size.y);

	// cleanup
	clReleaseMemObject(renderer->clProgram.sphereBuff);
	free(imageData);

	return image;
}

void render_to_file(Renderer *renderer, int samples, char *fileName, int verbose) {
	Image *image = render(renderer, samples, verbose);

	write_image(image, fileName);
	
	destroy_image(image);
}

void destroy_renderer(Renderer *renderer) {
	clReleaseMemObject(renderer->clProgram.imageBuff);
	clReleaseProgram(renderer->clProgram.program);
	clReleaseKernel(renderer->clProgram.kernel);
	clReleaseCommandQueue(renderer->clProgram.queue);
	clReleaseContext(renderer->clProgram.context);


}

void write_image(Image *image, char *fileName) {
	char *format = get_filename_ext(fileName);

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