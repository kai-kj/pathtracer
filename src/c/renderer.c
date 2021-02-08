#include "renderer.h"

void add_sphere(Sphere *sphereList, int *sphereCountPtr, float x, float y, float z, float radius, Material material) {
	cl_float3 center = {.x = x, .y = y, .z = z};

	Sphere sphere = (Sphere){center, radius, material};
	int sphereCount = *sphereCountPtr + 1;

	sphereList = realloc(sphereList, sizeof(Sphere) * sphereCount);
	sphereList[sphereCount - 1] = sphere;

	*sphereCountPtr = sphereCount;
}

Material create_lambertian_material(Color color) {
	return (Material){1, rgb_to_float3(color), 0, 0, 0};
}

Material create_metal_material(Color color,  float tint, float fuzzyness) {
	return (Material){2, rgb_to_float3(color), tint, fuzzyness, 0};
}

Material create_dielectric_material(Color color, float tint, float fuzzyness, float refIdx) {
	return (Material){3, rgb_to_float3(color), tint, fuzzyness, refIdx};
}

Material create_light_source_material(Color color) {
	return (Material){0, rgb_to_float3(color), 0, 0, 0};
}

cl_float3 *render(SceneInfo sceneInfo, Camera camera, Sphere *sphereList, int verbose) {
	int pixelCount = camera.resolution.x * camera.resolution.y;

	//---- initialise opencl -------------------------------------------------//

	CLProgram clProgram = create_cl_program("src/cl/main.cl", "main");

	//---- setup buffers -----------------------------------------------------//

	// create buffers
	cl_mem imageBuff = clCreateBuffer(clProgram.context, CL_MEM_READ_WRITE, sizeof(cl_float3) * pixelCount, NULL, NULL);
	cl_mem sphereBuff = clCreateBuffer(clProgram.context, CL_MEM_READ_ONLY, sizeof(Sphere) * sceneInfo.sphereCount, NULL, NULL);

	// copy data to buffers
	clEnqueueWriteBuffer(clProgram.queue, sphereBuff, CL_TRUE, 0, sizeof(Sphere) * sceneInfo.sphereCount, sphereList, 0, NULL, NULL);

	// set permanent kernel arguments
	clSetKernelArg(clProgram.kernel, 0, sizeof(cl_mem), &imageBuff);
	clSetKernelArg(clProgram.kernel, 1, sizeof(SceneInfo), &sceneInfo);
	clSetKernelArg(clProgram.kernel, 2, sizeof(Camera), &camera);
	clSetKernelArg(clProgram.kernel, 3, sizeof(cl_mem), &sphereBuff);

	//---- render image ------------------------------------------------------//

	double renderStartTime = get_time();

	size_t workItems = camera.resolution.x * camera.resolution.y;

	for(cl_int frame = 0; frame < camera.samples; frame++) {
		// seed
		cl_ulong seed = rand();

		// kernel arguments
		clSetKernelArg(clProgram.kernel, 4, sizeof(cl_int), &frame);
		clSetKernelArg(clProgram.kernel, 5, sizeof(cl_ulong), &seed);

		// run program
		run_cl_program(clProgram, workItems, 1);

		// print time
		if(verbose && frame % 10 == 9) {
			float frameTime = (get_time() - renderStartTime) / (frame + 1);
			float et = (camera.samples - (frame * 1)) * frameTime;
			int h = sec_to_h(et);
			int min = sec_to_min(et) - h * 60;
			int sec = et - h * 360 - min * 60;
			msg("\r%d/%d samples (%f%%), ET: %02d:%02d:%02d", frame + 1, camera.samples, (float)(frame + 1) / camera.samples * 100, h, min, sec);
		}
	}

	//---- read image --------------------------------------------------------//
	cl_float3 *image = malloc(sizeof(cl_float3) * pixelCount);
	clEnqueueReadBuffer(clProgram.queue, imageBuff, CL_TRUE, 0, sizeof(cl_float3) * pixelCount, image, 0, NULL, NULL);

	//---- cleanup opencl ----------------------------------------------------//
	clReleaseMemObject(imageBuff);
	clReleaseMemObject(sphereBuff);
	
	destroy_cl_program(clProgram);

	return image;

}