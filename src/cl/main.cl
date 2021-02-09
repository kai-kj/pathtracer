#include "rng.cl"
#include "ray.cl"

kernel void main(global float3 *image, ImageInfo imageInfo, SceneInfo sceneInfo, Camera camera, global Sphere *sphereList, int frame, ulong seed) {
	int id = (int)get_global_id(0);
	ulong rng = init_rng_2(id, seed);
	
	int row = id / imageInfo.size.x;
	int column = id % imageInfo.size.x;

	float aspectRatio = (float)imageInfo.size.x / (float)imageInfo.size.y;

	float xOffset = 2 * (float)(column - imageInfo.size.x / 2) / (float)imageInfo.size.x * camera.sensorWidth;
	float yOffset = 2 * (float)(row - imageInfo.size.y / 2) / (float)imageInfo.size.y * camera.sensorWidth / aspectRatio;
	float3 offset = (float3){-xOffset, yOffset, camera.focalLength};

	float3 origin = camera.pos + rotate_vector(offset, camera.rot);

	float3 target = camera.pos + random_unit_vector(&rng) * camera.aperture;

	float3 direction = normalize(target - origin);

	Ray ray = (Ray){camera.pos, direction};

	float3 color = cast_ray(sceneInfo, sphereList, ray, imageInfo.maxRayDepth, &rng);

	color = color * camera.exposure * camera.aperture;

	image[id] = (image[id] * frame + color) / (frame + 1);
}
