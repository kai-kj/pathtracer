#include "src/cl/ray.cl"

float3 cast_ray(Ray ray, global int *voxels, SceneInfo sceneInfo) {
	int3 step;

	if(ray.direction.x > 0) step.x = 1;
	else if(ray.direction.x < 0) step.x = -1;
	else step.x = 0;

	if(ray.direction.y > 0) step.y = 1;
	else if(ray.direction.y < 0) step.y = -1;
	else step.y = 0;

	if(ray.direction.z > 0) step.z = 1;
	else if(ray.direction.z < 0) step.z = -1;
	else step.z = 0;

	float3 tMax;

	if(ray.direction.x != 0) tMax.x = (ceil(ray.origin.x) - ray.origin.x) / ray.direction.x;
	else tMax.x = INFINITY;

	if(ray.direction.y != 0) tMax.y = (ceil(ray.origin.y) - ray.origin.y) / ray.direction.y;
	else tMax.y = INFINITY;

	if(ray.direction.z != 0) tMax.z = (ceil(ray.origin.z) - ray.origin.z) / ray.direction.z;
	else tMax.z = INFINITY;

	float3 tDelta = (float3) {
		1 / ray.direction.x,
		1 / ray.direction.y,
		1 / ray.direction.z
	};

	int side;
	float3 hitPos;
	int3 worldPos = (int3){ray.origin.x, ray.origin.y, ray.origin.z};

	while(1) {
		if(tMax.x < tMax.y) {
			if(tMax.x < tMax.z) {
				worldPos.x += step.x;
				tMax.x += tDelta.x;
				side = 1;
			} else {
				worldPos.z += step.z;
				tMax.z += tDelta.z;
				side = 3;
			}
			} else {
				if(tMax.y < tMax.z) {
				worldPos.y += step.y;
				tMax.y += tDelta.y;
				side = 2;
			} else {
				worldPos.z += step.z;
				tMax.z += tDelta.z;
				side = 3;
			}
		}

		if(worldPos.x < 0 || worldPos.x > sceneInfo.size.x || worldPos.y < 0 || worldPos.y > sceneInfo.size.y || worldPos.z < 0 || worldPos.z > sceneInfo.size.z) break;

		if(voxels[worldPos.x + worldPos.y * sceneInfo.size.x + worldPos.z * sceneInfo.size.x * sceneInfo.size.y] != 0) {
			if(side == 1) {
				hitPos.x = worldPos.x;
				hitPos.y = ray.origin.y + (hitPos.x - ray.origin.x) * ray.direction.y / ray.direction.x;
				hitPos.z = ray.origin.z + (hitPos.z - ray.origin.z) * ray.direction.z / ray.direction.x;
			} else if(side == 2) {
				hitPos.y = worldPos.y;
				hitPos.x = ray.origin.x + (hitPos.y - ray.origin.y) * ray.direction.x / ray.direction.y;
				hitPos.z = ray.origin.z + (hitPos.z - ray.origin.z) * ray.direction.z / ray.direction.y;
			} else if(side == 3) {
				hitPos.z = worldPos.z;
				hitPos.y = ray.origin.y + (hitPos.x - ray.origin.x) * ray.direction.y / ray.direction.z;
				hitPos.x = ray.origin.x + (hitPos.y - ray.origin.y) * ray.direction.x / ray.direction.z;
			}

			float distance = length(hitPos - ray.origin);

			return distance / 10;
		}
	}

	return 0;
}

kernel void main(global float3 *image, ImageInfo imageInfo, global int *voxels, SceneInfo sceneInfo, Camera camera) {
	int id = get_global_id(0);

	int row = id / imageInfo.size.x;
	int column = id % imageInfo.size.x;

	float aspectRatio = (float)imageInfo.size.x / (float)imageInfo.size.y;

	float xOffset = 2 * (float)(column - imageInfo.size.x / 2) / (float)imageInfo.size.x * camera.sensorWidth;
	float yOffset = 2 * (float)(row - imageInfo.size.y / 2) / (float)imageInfo.size.y * camera.sensorWidth / aspectRatio;
	float3 offset = (float3){-xOffset, yOffset, camera.focalLength};

	float3 origin = camera.pos + rotate_vector(offset, camera.rot);

	float3 target = camera.pos;

	float3 direction = normalize(target - origin);

	Ray ray = (Ray){camera.pos, direction};
	
	image[id] = cast_ray(ray, voxels, sceneInfo) * camera.exposure * camera.aperture;
}
