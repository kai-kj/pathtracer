#include "math.cl"
#include "material.cl"

typedef struct Camera {
	float3 pos;
	float3 rot;
	float sensorWidth;
	float focalLength;
	float aperture;
	float exposure;
} Camera;

int out_of_scene(int3 sceneSize, int3 pos) {
	return pos.x < 0 || pos.x >= sceneSize.x || pos.y < 0 || pos.y >= sceneSize.y || pos.z < 0 || pos.z >= sceneSize.z;
}

int get_voxel_id(global int *voxels, int3 sceneSize, int3 pos) {
	return voxels[pos.x + pos.y * sceneSize.x + pos.z * sceneSize.x * sceneSize.y];
}

float3 cast_ray(Ray ray, global int *voxels, int3 sceneSize, global Material *materials, float3 bgColor) {
	int3 pos = {
		(int)floor(ray.origin.x),
		(int)floor(ray.origin.y),
		(int)floor(ray.origin.z)
	};

	float3 step = {
		(ray.direction.x >= 0) ? 1 : -1,
		(ray.direction.y >= 0) ? 1 : -1,
		(ray.direction.z >= 0) ? 1 : -1
	};

	float3 tMax = {
		(ray.direction.x != 0) ? (pos.x + step.x - ray.origin.x) / ray.direction.x : MAXFLOAT,
		(ray.direction.y != 0) ? (pos.y + step.y - ray.origin.y) / ray.direction.y : MAXFLOAT,
		(ray.direction.z != 0) ? (pos.z + step.z - ray.origin.z) / ray.direction.z : MAXFLOAT
	};
	

	float3 tDelta = {
		(ray.direction.x != 0) ? 1 / ray.direction.x * step.x : MAXFLOAT,
		(ray.direction.y != 0) ? 1 / ray.direction.y * step.y : MAXFLOAT,
		(ray.direction.z != 0) ? 1 / ray.direction.z * step.z : MAXFLOAT
	};
	
	int side;
	float3 hitPos;

	while(1) {
		if(tMax.x < tMax.y) {
			if(tMax.x < tMax.z) {
				pos.x += step.x;
				tMax.x += tDelta.x;
				side = 1;
			} else {
				pos.z += step.z;
				tMax.z += tDelta.z;
				side = 3;
			}
		} else {
			if(tMax.y < tMax.z) {
				pos.y += step.y;
				tMax.y += tDelta.y;
				side = 2;
			} else {
				pos.z += step.z;
				tMax.z += tDelta.z;
				side = 3;
			}
		}

		if(out_of_scene(sceneSize, pos))
			break;

		int id = get_voxel_id(voxels, sceneSize, pos);

		if(id != 0) {
			if(side == 1) {
				hitPos.x = (float)pos.x;
				hitPos.y = ray.origin.y + (hitPos.x - ray.origin.x) * ray.direction.y / ray.direction.x;
				hitPos.z = ray.origin.z + (hitPos.x - ray.origin.x) * ray.direction.z / ray.direction.x;
			} else if(side == 2) {
				hitPos.y = (float)pos.y;
				hitPos.x = ray.origin.x + (hitPos.y - ray.origin.y) * ray.direction.x / ray.direction.y;
				hitPos.z = ray.origin.z + (hitPos.y - ray.origin.y) * ray.direction.z / ray.direction.y;
			} else if(side == 3) {
				hitPos.z = (float)pos.z;
				hitPos.y = ray.origin.y + (hitPos.z - ray.origin.z) * ray.direction.y / ray.direction.z;
				hitPos.x = ray.origin.x + (hitPos.z - ray.origin.z) * ray.direction.x / ray.direction.z;
			}

			float distance = length(hitPos - ray.origin);
			
			return materials[id].color;
			return (float3){(float)hitPos.x / (float)sceneSize.x, (float)hitPos.y / (float)sceneSize.y, (float)hitPos.z / (float)sceneSize.z};

		}
	}

	return 0;
}

kernel void main(global float3 *image, int2 imageSize, global int *voxels, int3 sceneSize, global Material *materials, int materialCount, float3 bgColor, Camera camera) {
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
	
	image[id] = cast_ray(ray, voxels, sceneSize, materials, bgColor) * camera.exposure * camera.aperture;
}
