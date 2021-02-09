typedef struct Ray {
	float3 origin;
	float3 direction;
} Ray;

typedef struct ImageInfo {
	int2 size;
	int maxRayDepth;
} ImageInfo;

typedef struct SceneInfo {
	float3 color;
	int sphereCount;
} SceneInfo;

typedef struct Material {
	int type;
	float3 color;
	float tint;
	float fuzzyness;
	float refIdx;
} Material;

typedef struct Sphere {
	float3 center;
	float radius;
	Material material;
} Sphere;

typedef struct Camera {
	float3 pos;
	float3 rot;
	float sensorWidth;
	float focalLength;
	float aperture;
	float exposure;
} Camera;

float3 rotate_vector(float3 vec, float3 rot) {
	float3 nVec;
	
	nVec.x = vec.x * cos(rot.z) - vec.y * sin(rot.z);
	nVec.y = vec.x * sin(rot.z) + vec.y * cos(rot.z);

	nVec.x = vec.x * cos(rot.y) + vec.z * sin(rot.y);
	nVec.z = -vec.x * sin(rot.y) + vec.z * cos(rot.y);

	nVec.y = vec.y * cos(rot.x) - vec.z * sin(rot.x);
	nVec.z = vec.y * sin(rot.x) + vec.z * cos(rot.x);

	return nVec;
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
	float c = oc.x * oc.x + oc.y * oc.y + oc.z * oc.z - sphere.radius * sphere.radius;
	float discriminant = bHalf * bHalf - a * c;

	if(discriminant < 0)
		return 0;
	
	*distance = (-bHalf - sqrt(discriminant)) / a;

	if(*distance > 0)
		return 1;
	else
		return 0;
}

float3 cast_ray(SceneInfo sceneInfo, global Sphere *sphereList, Ray firstRay, int maxDepth, ulong *rng) {
	Ray ray = firstRay;

	float3 color = 0;
	float3 mask = 1;

	int j;
	for(j = 0; j < maxDepth; j++) {
		Sphere sphere;
		bool hit = false;
		float distance;

		for(int i = 0; i < sceneInfo.sphereCount; i++) {
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
			color = mask * sceneInfo.color;
			break;
		}

		float3 hitPos = ray.origin + ray.direction * distance;
		float3 normal = normalize(hitPos - sphere.center);
		float3 offsettedHitPos = hitPos - normal * 0.0001f;
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
				float3 direction = normal + random_unit_vector(rng);
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