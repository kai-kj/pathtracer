//---- structs ---------------------------------------------------------------//

typedef struct Ray {
	float3 origin;
	float3 direction;
} Ray;

typedef struct Scene {
	float3 bgColor;
	int sphereCount;
} Scene;

typedef struct Camera {
	int width;
	int height;
	float fov;
	int samples;
	int maxDepth;
} Camera;

typedef struct Material {
	int type;
	float3 color;

	float reflectance;
	float fuzzyness;
	float refIdx;
} Material;

typedef struct Sphere {
	float3 center;
	float radius;
	Material material;
} Sphere;

//---- rng (not great, but works) --------------------------------------------//

ulong init_rng_1(ulong a) {
	return (16807 * a) % 2147483647 * (16807 * a) % 2147483647;
}

ulong init_rng_2(ulong a, ulong b) {
	return (16807 * a * b) % 2147483647 * (16807 * a * b) % 2147483647;
}

float rand_float(ulong *rng) {
	*rng = (16807 * *rng) % 2147483647;
	return *rng / 2147483647.0;
}

float rand_float_in_range(float a, float b, ulong *rng) {
	return a + rand_float(rng) * (b - a);
}

//---- small functions -------------------------------------------------------//

float3 random_unit_vector(ulong *rng) {
	while(1) {
		float3 r;
		r.x = rand_float_in_range(-1, 1, rng);
		r.y = rand_float_in_range(-1, 1, rng);
		r.z = rand_float_in_range(-1, 1, rng);
		
		if(pow(r.x, 2) + pow(r.y, 2) + pow(r.z, 2) < 1)
			return normalize(r);
		
	}
}

float3 reflection_dir(float3 in, float3 normal) {
	return in - normal * 2 * dot(in, normal);
}

float reflectance(float cosTheta, float relativeRefIdx) {
	float r = (1 - relativeRefIdx) / (1 + relativeRefIdx);
	r = r * r;
	return r + (1 - r) * pow((1 - cosTheta), 5);
}

float3 refraction_dir(float3 in, float3 normal, float relativeRefIdx) {
	float cosTheta = fmin(dot(in * -1, normal), 1);
	float3 outPerpendicular = (in + normal * cosTheta) * relativeRefIdx;
	float3 outParallel = normal * -sqrt(fabs(1 - length(outPerpendicular) * length(outPerpendicular)));
	return outParallel + outPerpendicular;
}

int hit_sphere(Ray ray, Sphere sphere, float *distance) {
    float3 oc = ray.origin - sphere.center;
    float a = pow(length(ray.direction), 2);
    float bHalf = dot(oc, ray.direction);
    float c = pow(oc.x, 2) + pow(oc.y, 2) + pow(oc.z, 2) - pow(sphere.radius, 2);
    float discriminant = pow(bHalf, 2) - a * c;

	if(discriminant < 0)
		return 0;
	
	*distance = (-bHalf - sqrt(discriminant)) / a;

	if(*distance > 0)
		return 1;
	else
		return 0;
}

//---- main ray function -----------------------------------------------------//

float3 cast_ray(Scene scene, global Sphere *sphereList, Ray firstRay, int maxDepth, ulong *rng) {
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

		if(!hit)
			break;
		
		float3 hitPos = ray.origin + ray.direction * distance;
		float3 normal = normalize(hitPos - sphere.center);
		float3 offsettedHitPos = hitPos + normal * FLT_EPSILON;
		bool front;

		if(dot(ray.direction, normal) > 0) {
			front = false;
			normal = normal * -1;
		
		} else {
			front = true;
		}
		
		if(sphere.material.type == 0) { // light source
			color = mask * sphere.material.color;
			break;

		} else if(sphere.material.type == 1) { // lambertian material
			float3 direction = offsettedHitPos + normal + random_unit_vector(rng) - offsettedHitPos;
			ray = (Ray){offsettedHitPos, direction};
			mask *= sphere.material.color;

		} else if(sphere.material.type == 2) { // metal materia
			float3 direction = reflection_dir(ray.direction, normal);
			ray = (Ray){offsettedHitPos, direction};
			// mask *= sphere.material.color

		} else if(sphere.material.type == 3) { // dielectric material
			float relativeRefIdx;

			if(front) /* entering material */
				relativeRefIdx = sphere.material.refIdx;
			else /* exiting material */
				relativeRefIdx = 1 / sphere.material.refIdx;
			
			float cosTheta = fmin(dot(ray.direction * -1, normal), 1);
			float sinTheta = sqrt(1 - pow(cosTheta, 2));
			
			float3 direction;

			if (relativeRefIdx * sinTheta > 1 || reflectance(cosTheta, relativeRefIdx) > rand_float(rng)) // ray can't refract
				direction = reflection_dir(ray.direction, normal);

			else // ray can refract
				direction = refraction_dir(ray.direction, normal, relativeRefIdx);

			ray = (Ray){offsettedHitPos, direction};
			// mask *= sphere.material.color
		}
	}

	return color;
}

//---- main ------------------------------------------------------------------//

kernel void main(global float3 *image, Scene scene, Camera camera, global Sphere *sphereList, int frame, global ulong *seedList) {
	int id = (int)get_global_id(0);
	ulong rng = init_rng_1(seedList[id]);
	
	
	int row = id / camera.width;
	int column = id % camera.width;

	float tanFOV = tan(camera.fov / 2);
	float aspectRatio = (float)camera.width / (float)camera.height;

	float y = - (2 * (row + rand_float(&rng)) / camera.height - 1) * tanFOV;
	float x = (2 * (column + rand_float(&rng)) / camera.width - 1) * tanFOV * aspectRatio;

	Ray ray = (Ray){(float3){0, 0, 0}, normalize((float3){x, y, -1})};

	float3 color = cast_ray(scene, sphereList, ray, camera.maxDepth, &rng);
	image[id] = (image[id] * frame + color) / (frame + 1);
	

	//if(id == 0) {
		//image[id] = random_unit_vector(&rng);
	//}

}
