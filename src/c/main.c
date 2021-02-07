#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <CL/cl.h>

#define k_cl_image_IMPLEMENTATION
#include "../../include/k_cl_image.h"

#define K_MATH_IMPLEMENTATION
#include "../../include/k_math.h"

#define K_UTIL_IMPLEMENTATION
#define K_LOG_PRINT_TO_SCREEN
#define K_LOG_PRINT_TO_FILE
#define K_LOG_FILE "log.txt"
#include "../../include/k_util.h"

#include "materials.h"
#include "scene.h"

#define WIDTH 400
#define HEIGHT 300
#define FOV PI / 3
#define SAMPLES 100
#define MAX_DEPTH 10

int main(void) {

	//---- initialise values -------------------------------------------------//
	int pixelCount = WIDTH * HEIGHT;

	cl_float3 *finalImage = malloc(sizeof(cl_float3) * pixelCount);

	for(int i = 0; i < pixelCount; i++) {
		finalImage[i].x = 0;
		finalImage[i].y = 0;
		finalImage[i].z = 0;
	}

	Scene scene = {
		.sphereCount = 0,
		.bgColor.x = 0,
		.bgColor.y = 0,
		.bgColor.z = 0
	};

	Camera camera = {
		.width = WIDTH,
		.height = HEIGHT,
		.fov = FOV,
		.samples = SAMPLES,
		.maxDepth = MAX_DEPTH
	};

	Sphere *sphereList = malloc(sizeof(Sphere));

	Material lightSource = create_light_source_material((Color){50, 50, 25});
	Material white = create_lambertian_material((Color){1, 1, 1}, 0.5);
	Material red = create_lambertian_material((Color){1, 0, 0}, 0.5);
	Material green = create_lambertian_material((Color){0, 1, 0}, 0.5);
	Material mirror = create_metal_material((Color){0, 0, 0}, 1, 0);
	Material glass = create_dielectric_material((Color){0, 0, 0}, 0.95, 0);

	scene.sphereCount = add_sphere(sphereList, scene.sphereCount, 0, 54.98, -10, 50, lightSource);
	scene.sphereCount = add_sphere(sphereList, scene.sphereCount, 0, 105, -10, 100, white);
	scene.sphereCount = add_sphere(sphereList, scene.sphereCount, 105, 0, -5, 100, green);
	scene.sphereCount = add_sphere(sphereList, scene.sphereCount, -105, 0, -5, 100, red);
	scene.sphereCount = add_sphere(sphereList, scene.sphereCount, 0, 0, -120, 100, white);
	scene.sphereCount = add_sphere(sphereList, scene.sphereCount, 0, -105, -10, 100, white);
	scene.sphereCount = add_sphere(sphereList, scene.sphereCount, -2.5, -3, -10, 2, glass);
	scene.sphereCount = add_sphere(sphereList, scene.sphereCount, 2, -4, -15, 1, mirror);

	//---- initialise opencl -------------------------------------------------//

	// get platforms
	cl_platform_id platform;
	cl_uint platformNum;
	clGetPlatformIDs(1, &platform, &platformNum);

	// get devices
	cl_device_id device;
	cl_uint deviceNum;
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 2, &device, &deviceNum);

	// create context and command queue
	cl_context context = clCreateContext(0, 1, &device, NULL, NULL, NULL);
	cl_command_queue commandQueue = clCreateCommandQueueWithProperties(context, device, 0, NULL);

	// load program
	const char *source = read_file("src/cl/main.cl");
	cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, NULL);

	// build program
	if(clBuildProgram(program, 0, NULL, NULL, NULL, NULL) != CL_SUCCESS) {
		printf("Failed to build program\n");

		char buffer[0x100000];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
		printf("%s\n", buffer);
		exit(1);
	}

	// create kernel object
	cl_kernel kernel = clCreateKernel(program, "main", NULL);

	//---- setup buffers -----------------------------------------------------//

	// random numbers
	srand(time(0));
	cl_ulong *baseOffsets = malloc(sizeof(cl_long) * pixelCount);

	// create buffers
	cl_mem imageBuff = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float3) * pixelCount, NULL, NULL);
	cl_mem sphereBuff = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(Sphere) * scene.sphereCount, NULL, NULL);
	cl_mem offsetBuff = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_long) * pixelCount, NULL, NULL);

	// copy data to buffers
	clEnqueueWriteBuffer(commandQueue, sphereBuff, CL_TRUE, 0, sizeof(Sphere) * scene.sphereCount, sphereList, 0, NULL, NULL);
	clEnqueueWriteBuffer(commandQueue, offsetBuff, CL_TRUE, 0, sizeof(cl_long) * pixelCount, baseOffsets, 0, NULL, NULL);

	//---- render image ------------------------------------------------------//

	msg("\nSTARTING RENDER (%dx%d, %d samples)\n\n", camera.width, camera.height, camera.samples);

	double startTime = get_time();

	cl_float3 *image = malloc(sizeof(cl_float3) * pixelCount);

	for(cl_int frame = 0; frame < 1; frame++) {
		// set kernel arguments
		for(int i = 0; i < pixelCount; i++) {
			baseOffsets[i] = rand();
		}

		clSetKernelArg(kernel, 0, sizeof(cl_mem), &imageBuff);
		clSetKernelArg(kernel, 1, sizeof(Scene), &scene);
		clSetKernelArg(kernel, 2, sizeof(Camera), &camera);
		clSetKernelArg(kernel, 3, sizeof(cl_mem), &sphereBuff);
		clSetKernelArg(kernel, 4, sizeof(cl_int), &frame);
		clSetKernelArg(kernel, 5, sizeof(cl_mem), &offsetBuff);

		// run program
		size_t workItems = camera.width * camera.height;
		cl_int ret = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &workItems, NULL, 0, NULL, NULL);

		switch(ret) {
			case CL_INVALID_PROGRAM_EXECUTABLE:
				printf("there is no successfully built program executable available for device associated with command_queue.\n");
				break;
			case CL_INVALID_COMMAND_QUEUE:
				printf("command_queue is not a valid host command-queue.\n");
				break;
			case CL_INVALID_KERNEL:
				printf("kernel is not a valid kernel object.\n");
				break;
			case CL_INVALID_CONTEXT:
				printf("context associated with command_queue and kernel are not the same or if the context associated with command_queue and events in event_wait_list are not the same.\n");
				break;
			case CL_INVALID_KERNEL_ARGS:
				printf("the kernel argument values have not been specified or if a kernel argument declared to be a pointer to a type does not point to a named address space.\n");
				break;
			case CL_INVALID_WORK_DIMENSION:
				printf("work_dim is not a valid value (i.e. a value between 1 and CL_​DEVICE_​MAX_​WORK_​ITEM_​DIMENSIONS).\n");
				break;
			case CL_INVALID_GLOBAL_WORK_SIZE:
				printf("any of the values specified in global_work_size[0], …​ global_work_size[work_dim - 1] exceed the maximum value representable by size_t on the device on which the kernel-instance will be enqueued.\n");
				printf("any of the values specified in global_work_size[0], …​ global_work_size[work_dim - 1] exceed the maximum value representable by size_t on the device on which the kernel-instance will be enqueued.\n");
				break;
			case CL_INVALID_GLOBAL_OFFSET:
				printf("the value specified in global_work_size + the corresponding values in global_work_offset for any dimensions is greater than the maximum value representable by size t on the device on which the kernel-instance will be enqueued.\n");
				break;
			case CL_INVALID_WORK_GROUP_SIZE:
				printf("local_work_size is specified and does not match the required work-group size for kernel in the program source.\n");
				printf("local_work_size is specified and is not consistent with the required number of sub-groups for kernel in the program source.\n");
				printf("local_work_size is specified and the total number of work-items in the work-group computed as local_work_size[0] × …​ local_work_size[work_dim - 1] is greater than the value specified by CL_​KERNEL_​WORK_​GROUP_​SIZE in the Kernel Object Device Queries table.\n");
				printf("the program was compiled with cl-uniform-work-group-size and the number of work-items specified by global_work_size is not evenly divisible by size of work-group given by local_work_size or by the required work-group size specified in the kernel source.\n");
				printf("the number of work-items specified in any of local_work_size[0], …​ local_work_size[work_dim - 1] is greater than the corresponding values specified by CL_​DEVICE_​MAX_​WORK_​ITEM_​SIZES[0], …​, CL_​DEVICE_​MAX_​WORK_​ITEM_​SIZES[work_dim - 1].\n");
				break;
			case CL_MISALIGNED_SUB_BUFFER_OFFSET:
				printf("a sub-buffer object is specified as the value for an argument that is a buffer object and the offset specified when the sub-buffer object is created is not aligned to CL_​DEVICE_​MEM_​BASE_​ADDR_​ALIGN value for device associated with queue. This error code is missing before version 1.1.\n");
				break;
			case CL_INVALID_IMAGE_SIZE:
				printf("an image object is specified as an argument value and the image dimensions (image width, height, specified or compute row and/or slice pitch) are not supported by device associated with queue.\n");
				break;
			case CL_IMAGE_FORMAT_NOT_SUPPORTED:
				printf("an image object is specified as an argument value and the image format (image channel order and data type) is not supported by device associated with queue.\n");
				break;
			case CL_OUT_OF_RESOURCES:
				printf("there is a failure to queue the execution instance of kernel on the command-queue because of insufficient resources needed to execute the kernel. For example, the explicitly specified local_work_size causes a failure to execute the kernel because of insufficient resources such as registers or local memory. Another example would be the number of read-only image args used in kernel exceed the CL_​DEVICE_​MAX_​READ_​IMAGE_​ARGS value for device or the number of write-only and read-write image args used in kernel exceed the CL_​DEVICE_​MAX_​READ_​WRITE_​IMAGE_​ARGS value for device or the number of samplers used in kernel exceed CL_​DEVICE_​MAX_​SAMPLERS for device.\n");
				break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				printf("there is a failure to allocate memory for data store associated with image or buffer objects specified as arguments to kernel.\n");
				break;
			case CL_INVALID_EVENT_WAIT_LIST:
				printf("event_wait_list is NULL and num_events_in_wait_list > 0, or event_wait_list is not NULL and num_events_in_wait_list is 0, or if event objects in event_wait_list are not valid events.\n");
				break;
			case CL_INVALID_OPERATION:
				printf("SVM pointers are passed as arguments to a kernel and the device does not support SVM or if system pointers are passed as arguments to a kernel and/or stored inside SVM allocations passed as kernel arguments and the device does not support fine grain system SVM allocations.\n");
				break;
			case CL_OUT_OF_HOST_MEMORY:
				printf("there is a failure to allocate resources required by the OpenCL implementation on the host.\n");
				break;
		}

		clEnqueueReadBuffer(commandQueue, imageBuff, CL_TRUE, 0, sizeof(cl_float3) * pixelCount, image, 0, NULL, NULL);

		for(int i = 0; i < pixelCount; i++) {
			finalImage[i].x = (finalImage[i].x * frame + image[i].x) / (frame + 1);
			finalImage[i].y = (finalImage[i].y * frame + image[i].y) / (frame + 1);
			finalImage[i].z = (finalImage[i].z * frame + image[i].z) / (frame + 1);
		}
	}

	double endTime = get_time();

	msg("\nFINISHED RENDER\n\n");

	// read image
	clEnqueueReadBuffer(commandQueue, imageBuff, CL_TRUE, 0, sizeof(cl_float3) * pixelCount, image, 0, NULL, NULL);

	/*
	printf("%f, %f, %f\n", image[0].x, image[0].y, image[0].z);
	printf("%f, %f, %f\n\n", image[1].x, image[1].y, image[1].z);
	*/

	// cleanup opencl
	clReleaseMemObject(imageBuff);
	clReleaseMemObject(sphereBuff);
	clReleaseMemObject(offsetBuff);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);

	//---- write image -------------------------------------------------------//

	printf("%f, %f, %f\n", image[0].x, image[0].y, image[0].z);
	printf("%f, %f, %f\n", image[1].x, image[1].y, image[1].z);

	gamma_correct_image(finalImage, pixelCount);
	write_image(finalImage, WIDTH, HEIGHT, "test.png");

	//---- cleanup -----------------------------------------------------------//

	float time = endTime - startTime;
	int h = sec_to_h(time);
	int min = sec_to_min(time) - h * 60;
	float sec = time - h * 3600 - min * 60;

	msg("\nDONE (%02d:%02d:%.2f)\n\n", h, min, sec);

	return 0;
}
