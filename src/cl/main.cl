#include "src/cl/ray.cl"

float3 cast_ray(Ray ray, global int *voxels, int3 sceneSize, float3 bgColor) {
	int3 worldPos = (int3){
		ray.origin.x,
		ray.origin.y,
		ray.origin.z
	};

	int3 step = (int3){
		(ray.direction.x > 0) ? 1 : (ray.direction.x < 0) ? -1 : 0,
		(ray.direction.y > 0) ? 1 : (ray.direction.y < 0) ? -1 : 0,
		(ray.direction.z > 0) ? 1 : (ray.direction.z < 0) ? -1 : 0
	};

	float3 tMax = (float3){
		(ceil(ray.origin.x) - ray.origin.x) / ray.direction.x,
		(ceil(ray.origin.y) - ray.origin.y) / ray.direction.y,
		(ceil(ray.origin.z) - ray.origin.z) / ray.direction.z
	};
	
	float3 tDelta = (float3){
		1 / ray.direction.x,
		1 / ray.direction.y,
		1 / ray.direction.z
	};

	int side;
	float3 hitPos;

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

		if(worldPos.x < 0 || worldPos.x >= sceneSize.x || worldPos.y < 0 || worldPos.y >= sceneSize.y || worldPos.z < 0 || worldPos.z >= sceneSize.z)
			break;

		if(voxels[worldPos.x + worldPos.y * sceneSize.x + worldPos.z * sceneSize.x * sceneSize.y] != 0) {
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

			return (float3){(float)worldPos.x / (float)sceneSize.x, (float)worldPos.y / (float)sceneSize.y, (float)worldPos.z / (float)sceneSize.z};
			
			/*
			if((worldPos.x) % 2 == 0)
				return 1;
			else
				return 0.5;
			*/
		}
	}

	return 0;
}

kernel void main(global float3 *image, int2 imageSize, global int *voxels, int3 sceneSize, float3 bgColor, Camera camera) {
	int id = get_global_id(0);

	int row = id / imageSize.x;
	int column = id % imageSize.x;

	float aspectRatio = (float)imageSize.x / (float)imageSize.y;

	float xOffset = 2 * (float)(column - imageSize.x / 2) / (float)imageSize.x * camera.sensorWidth;
	float yOffset = 2 * (float)(row - imageSize.y / 2) / (float)imageSize.y * camera.sensorWidth / aspectRatio;
	float3 offset = (float3){xOffset, yOffset, camera.focalLength};

	float3 origin = camera.pos + rotate_vector(offset, camera.rot);

	float3 target = camera.pos;

	float3 direction = -normalize(target - origin);

	Ray ray = (Ray){camera.pos, direction};
	
	image[id] = cast_ray(ray, voxels, sceneSize, bgColor)/* * camera.exposure * camera.aperture*/;

	
	if(id == 0)
		image[id] = cast_ray(ray, voxels, sceneSize, bgColor) * 10;

	/*
	if(id == 1)
		image[id] = camera.rot;
	*/
}
