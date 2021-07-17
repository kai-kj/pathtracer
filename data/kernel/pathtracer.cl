#define PI 3.14159265359f

#define MATERIAL_TYPE_EMPTY 0
#define MATERIAL_TYPE_LIGHT_SOURCE 1
#define MATERIAL_TYPE_LAMBERTIAN 2
#define MATERIAL_TYPE_METAL 3
#define MATERIAL_TYPE_DIELECTRIC 4

//---- structs ---------------------------------------------------------------//

// TODO: char -> ushort

typedef struct Ray {
	float3 origin;
	float3 direction;
} Ray;

// TODO: attenuation for diffuse material
typedef struct Material {
	uchar type;
	float3 color;

	union {
		struct {
			float brightness;
		} lightSource;

		struct {
		} lambertian;

		struct {
			float tint;
			float fuzz;
		} metal;

		struct {
			float tint;
			float fuzz;
			float refIdx;
		} dielectric;
	} details;
} Material;

typedef struct Camera {
	float3 pos;
	float3 rot;
	float sensorWidth;
	float focalLength;
	float aperture;
	float exposure;
} Camera;

typedef struct Voxel {
	int3 pos;
	Material material;
} Voxel;

typedef struct Renderer {
	int2 imageSize;
	global float3 *image;
	int voxelCount;
	global Voxel *voxels;
	float3 bgColor;
	Camera camera;
	ulong *rng;
} Renderer;

//---- pesudo rng ------------------------------------------------------------//

ulong init_rng_1(ulong a) {
	return (16807 * a) % 2147483647 * (16807 * a) % 2147483647;
}

ulong init_rng_2(ulong a, ulong b) {
	return (16807 * a * b) % 2147483647 * (16807 * a * b) % 2147483647;
}

float get_rand_fraction(ulong *rng) {
	*rng = (16807 * *rng) % 2147483647;
	return *rng / 2147483647.0f;
}

float get_rand_float_in_range(ulong *rng, float a, float b) {
	return a + get_rand_fraction(rng) * (b - a);
}

float3 get_random_unit_vector(ulong *rng) {
	float theta = get_rand_float_in_range(rng, 0, 2 * PI);
	float z = get_rand_float_in_range(rng, -1, 1);
	float sqrtZ = sqrt(1 - z * z);

	return (float3){
		sqrtZ * cos(theta),
		sqrtZ * sin(theta),
		z
	};
}

#define get_max_idx(t, a, b) (t[a] > t[b] ? a : b)
#define get_min_idx(t, a, b) (t[a] < t[b] ? a : b)

//---- math ------------------------------------------------------------------//

float3 rotate_vector_x(float3 vec, float rot) {
	return (float3){
		vec.x,
		vec.y * cos(rot) - vec.z * sin(rot),
		vec.y * sin(rot) + vec.z * cos(rot)
	};
}

float3 rotate_vector_y(float3 vec, float rot) {
	return (float3){
		vec.x * cos(rot) + vec.z * sin(rot),
		vec.y,
		-vec.x * sin(rot) + vec.z * cos(rot)
	};
}

float3 rotate_vector_z(float3 vec, float rot) {
	return (float3){
		vec.y * cos(rot) - vec.z * sin(rot),
		vec.y * sin(rot) + vec.z * cos(rot),
		vec.z
	};
}

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

float3 get_reflection_dir(float3 in, float3 normal) {
	return in - normal * 2 * dot(in, normal);
}

float3 get_refraction_dir(float3 in, float3 normal, float relativeRefIdx) {
	float cosTheta = fmin(dot(in * -1, normal), 1);
	float3 outPerpendicular = (in + normal * cosTheta) * relativeRefIdx;
	float3 outParallel = normal * -sqrt(fabs(1 - length(outPerpendicular) * length(outPerpendicular)));
	return outParallel + outPerpendicular;
}

float get_reflectance(float cosTheta, float relativeRefIdx) {
	float r = (1 - relativeRefIdx) / (1 + relativeRefIdx);
	r = r * r;
	return r + (1 - r) * pow((1 - cosTheta), 5);
}

bool ray_voxel(Ray ray, Voxel voxel, float3 dirFrac, float *tMin) {
	float t[6];
	t[0] = (voxel.pos.x - ray.origin.x) * dirFrac.x;
	t[1] = t[0] + dirFrac.x;
	t[2] = (voxel.pos.y - ray.origin.y) * dirFrac.y;
	t[3] = t[2] + dirFrac.y;
	t[4] = (voxel.pos.z - ray.origin.z) * dirFrac.z;
	t[5] = t[4] + dirFrac.z;

	*tMin = max(max(min(t[0], t[1]), min(t[2], t[3])), min(t[4], t[5]));
	float tMax = min(min(max(t[0], t[1]), max(t[2], t[3])), max(t[4], t[5]));

	return tMax > *tMin && tMax >= 0;
}

constant int3 returnValues[6] = {
	(int3){-1, 0, 0},
	(int3){1, 0, 0},
	(int3){0, -1, 0},
	(int3){0, 1, 0},
	(int3){0, 0, -1},
	(int3){0, 0, 1}
};

int3 get_ray_box_normal(Ray ray, Voxel voxel, float3 dirFrac) {
	float t[6];

	t[0] = (voxel.pos.x - ray.origin.x) * dirFrac.x;
	t[1] = t[0] + dirFrac.x;
	t[2] = (voxel.pos.y - ray.origin.y) * dirFrac.y;
	t[3] = t[2] + dirFrac.y;
	t[4] = (voxel.pos.z - ray.origin.z) * dirFrac.z;
	t[5] = t[4] + dirFrac.z;

	int tMinIdx = get_max_idx(t, get_max_idx(t, get_min_idx(t, 0, 1), get_min_idx(t, 2, 3)), get_min_idx(t, 4, 5));

	return returnValues[tMinIdx];
}

//---- ray -------------------------------------------------------------------//

bool cast_ray(Renderer *r, Ray ray, float3 *hitPos, int3 *normal, Material *material) {
	bool hit = false;
	float minDist;
	int minIdx = -1;

	float3 dirFrac = (float3){
		(ray.direction.x != 0) ? (1.0f / ray.direction.x) : FLT_MAX,
		(ray.direction.y != 0) ? (1.0f / ray.direction.y) : FLT_MAX,
		(ray.direction.z != 0) ? (1.0f / ray.direction.z) : FLT_MAX
	};

	for(uint i = 0; i < r->voxelCount; i++) {
		float t;

		if(ray_voxel(ray, r->voxels[i], dirFrac, &t)) {
			if(!hit || t < minDist) {
				hit = true;
				minDist = t;
				minIdx = i;
			}
		}
	}

	if(!hit) return false;

	*hitPos = ray.origin + ray.direction * minDist;
	*normal = get_ray_box_normal(ray, r->voxels[minIdx], dirFrac);
	*material = r->voxels[minIdx].material;

	return true;
}

float3 get_color(Renderer *r, Ray ray, int maxDepth) {
	// TODO: maxDepth
	// TODO: russian rulette

	int returnFlag = false;
	float3 mask = 1;
	float3 color = 0;
	
	for(int i = 0; i < maxDepth; i++) {
		float3 hitPos;
		int3 iNormal;
		Material material;
		
		if(cast_ray(r, ray, &hitPos, &iNormal, &material)) {
			float3 fNormal = convert_float3(iNormal);
			hitPos += fNormal * 0.01f;

			// TODO: dielectric material
			switch(material.type) {
				case MATERIAL_TYPE_LIGHT_SOURCE:
					color = mask * material.color * material.details.lightSource.brightness;
					returnFlag = true;
					break;
				
				case MATERIAL_TYPE_LAMBERTIAN:
					ray = (Ray){
						hitPos,
						normalize(fNormal + get_random_unit_vector(r->rng))
					};
					mask *= material.color;
					break;
				
				case MATERIAL_TYPE_METAL:
					ray = (Ray){
						hitPos,
						normalize(get_reflection_dir(ray.direction, fNormal) + get_random_unit_vector(r->rng) * material.details.metal.fuzz)
					};
					mask = mask * (1 - material.details.metal.tint) + mask * material.color * material.details.metal.tint;
					break;
				
				default:
					break;

			}

		} else {
			color = mask * r->bgColor;
			returnFlag = true;
		
		}

		if(returnFlag) break;

	}

	return color;
}

//---- main ------------------------------------------------------------------//

// TODO: better sampling

Ray get_first_ray(Renderer *r, int id) {
	int row = id / r->imageSize.x;
	int column = id % r->imageSize.x;

	float aspectRatio = (float)r->imageSize.x / (float)r->imageSize.y;

	float xOffset = 2 * (float)(column - r->imageSize.x / 2) / (float)r->imageSize.x * r->camera.sensorWidth;
	float yOffset = 2 * (float)(row - r->imageSize.y / 2) / (float)r->imageSize.y * r->camera.sensorWidth / aspectRatio;
	float3 offset = (float3){xOffset, yOffset, r->camera.focalLength};

	float3 origin = r->camera.pos + rotate_vector(offset, r->camera.rot);

	float3 target = r->camera.pos;

	float3 direction = -normalize(target - origin);

	return (Ray){r->camera.pos, direction};
}

float3 adjust_color(Renderer *r, float3 color) {
	return color * r->camera.exposure * r->camera.aperture;
}

kernel void renderer(
	int2 imageSize, global float3 *image,
	int voxelCount, global Voxel *voxels,
	float3 bgColor,
	Camera camera,
	int sampleNumber,
	ulong seed
) {
	int id = get_global_id(0);
	ulong rng = init_rng_2(id, seed);

	Renderer r = (Renderer){
		imageSize, image,
		voxelCount, voxels,
		bgColor,
		camera,
		&rng
	};

	Ray ray = get_first_ray(&r, id);

	float3 color = get_color(&r, ray, 10);
	color = adjust_color(&r, color);
	
	image[id] = (image[id] * sampleNumber + color) / (sampleNumber + 1);

}
