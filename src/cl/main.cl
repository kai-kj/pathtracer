#include "rng.cl"
#include "math.cl"

float3 cast_ray(SceneInfo scene, global Sphere *sphereList, Ray firstRay, int maxDepth, ulong *rng) {
	Ray ray = firstRay;

	float3 color = 0;
	float3 mask = 1;

	int j;
	for(j = 0; j < maxDepth; j++) {
		Sphere sphere;
		bool hit = false;
		float distance;

		for(int i = 0; i < scene.sphereCount; i++) {
			float newDistance;
			if(hit_sphere(ray, sphereList[i], &newDistance)) {
				if(!hit || newDistance < distance) {
					hit = true;
					distance = newDistance;
					sphere = sphereList[i];
				}
			}
		}

		if(!hit) {
			color = mask * scene.bgColor;
			break;
		}

		float3 hitPos = ray.origin + ray.direction * distance;
		float3 normal = normalize(hitPos - sphere.center);
		float3 offsettedHitPos = hitPos + normal * -EPSILSON;
		bool front;

		if(dot(ray.direction, normal) > 0) {
			front = false;
			normal = normal * -1;
		
		} else {
			front = true;
		}
		
		if(sphere.material.type == 0) { // light source
			if(front) {
				color = mask * sphere.material.color;
				break;
			}

		} else if(sphere.material.type == 1) { // lambertian material
			if(front) {
				float3 direction = offsettedHitPos + normal + random_unit_vector(rng) - offsettedHitPos;
				ray = (Ray){offsettedHitPos, direction};
				mask *= sphere.material.color;
			}

		} else if(sphere.material.type == 2) { // metal material
			if(front) {
				float3 direction = reflection_dir(ray.direction, normal) + random_unit_vector(rng) * sphere.material.fuzzyness;
				ray = (Ray){offsettedHitPos, direction};
				mask = mask * (1 - sphere.material.tint) + mask * sphere.material.color * sphere.material.tint;
			}

		} else if(sphere.material.type == 3) { // dielectric material
			float relativeRefIdx;

			if(front) // entering material
				relativeRefIdx = sphere.material.refIdx;
			else // exiting material
				relativeRefIdx = 1 / sphere.material.refIdx;
			
			float cosTheta = fmin(dot(-ray.direction, normal), 1);
			float sinTheta = sqrt(1 - cosTheta * cosTheta);
			
			float3 direction;

			if (relativeRefIdx * sinTheta > 1 || reflectance(cosTheta, relativeRefIdx) > rand_float(rng)) // ray can't refract
				direction = reflection_dir(ray.direction, normal) + random_unit_vector(rng) * sphere.material.fuzzyness;

			else // ray can refract
				direction = refraction_dir(ray.direction, normal, relativeRefIdx) + random_unit_vector(rng) * sphere.material.fuzzyness;

			ray = (Ray){offsettedHitPos, direction};
			mask = mask * (1 - sphere.material.tint) + mask * sphere.material.color * sphere.material.tint;
		}
	}

	return color;
}

//---- main ------------------------------------------------------------------//

kernel void main(global float3 *image, SceneInfo scene, Camera camera, global Sphere *sphereList, int frame, ulong seed) {
	int id = (int)get_global_id(0);
	ulong rng = init_rng_2(id, seed);

	if(!camera.useDOF)
		camera.aperture = 0;
	
	int row = id / camera.resolution.x;
	int column = id % camera.resolution.x;

	float aspectRatio = (float)camera.resolution.x / (float)camera.resolution.y;

	float xOffset = 2 * (float)(column - camera.resolution.x / 2) / (float)camera.resolution.x * camera.sensorWidth;
	float yOffset = 2 * (float)(row - camera.resolution.y / 2) / (float)camera.resolution.y * camera.sensorWidth / aspectRatio;

	float3 origin = camera.pos + (float3){-xOffset, yOffset, camera.focalLength}; 
	float3 target = camera.pos + random_unit_vector(&rng) * camera.aperture;

	float3 camPosToOrigin = rotate_vector(origin - camera.pos, camera.rot);
	origin = camera.pos + camPosToOrigin;

	float3 direction = normalize(target - origin);

	Ray ray = (Ray){origin, direction};

	float3 color = cast_ray(scene, sphereList, ray, camera.maxRayDepth, &rng);

	if(camera.useDOF)
		color = color * camera.exposure * camera.aperture;

	image[id] = (image[id] * frame + color) / (frame + 1);
}
