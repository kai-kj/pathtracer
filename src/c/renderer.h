#include <CL/cl.h>

#include "../../include/k_util.h"
#include "../../include/k_cl_image.h"

#include "opencl.h"

typedef struct Material {
	cl_int type;
	cl_float3 color;

	cl_float tint;
	cl_float fuzzyness;
	cl_float refIdx;
} Material;

typedef struct Sphere {
	cl_float3 center;
	cl_float radius;
	Material material;

} Sphere;

typedef struct SceneInfo {
	cl_float3 bgColor;
	cl_int sphereCount;
} SceneInfo;

typedef struct Camera {
	cl_float3 pos;
	cl_float3 rot;
	cl_int2 resolution;
	cl_int useDOF;
	cl_float sensorWidth;
	cl_float focalLength;
	cl_float aperture;
	cl_float exposure;
	cl_int samples;
	cl_int maxRayDepth;
} Camera;

Material create_lambertian_material(Color color);
Material create_metal_material(Color color, float tint, float fuzzyness);
Material create_dielectric_material(Color color, float tint, float fuzzyness, float refIdx);
Material create_light_source_material(Color color);
void add_sphere(Sphere *sphereList, int *sphereCountPtr, float x, float y, float z, float radius, Material material);
cl_float3 *render(SceneInfo sceneInfo, Camera camera, Sphere *sphereList, int verbose);