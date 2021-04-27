//---- structs ---------------------------------------------------------------//

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

typedef int MaterialID;

typedef struct Camera {
	float3 pos;
	float3 rot;
	float sensorWidth;
	float focalLength;
	float aperture;
	float exposure;
} Camera;

// TODO: use unsigned variables or chars
typedef struct Renderer {
	global float3 *image;
	int2 imageSize;
	global unsigned char *voxels;
	int3 sceneSize;
	global Material *materials;
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

// float3 rotate_vector(float3 vec, float3 rot) {
// 	float3 nVec = vec;
	
// 	nVec = rotate_vector_x(nVec, rot.x);
// 	nVec = rotate_vector_y(nVec, rot.y);
// 	nVec = rotate_vector_z(nVec, rot.z);

// 	return nVec;
// }

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

//---- scene -----------------------------------------------------------------//

int get_voxel_index(Renderer *r, int3 voxel) {
	return voxel.x + voxel.y * r->sceneSize.x + voxel.z * r->sceneSize.x * r->sceneSize.y;
}

bool is_out_of_scene(Renderer *r, int3 voxel) {
	return voxel.x < 0 || voxel.x >= r->sceneSize.x || voxel.y < 0 || voxel.y >= r->sceneSize.y || voxel.z < 0 || voxel.z >= r->sceneSize.z;
}

//---- material --------------------------------------------------------------//

MaterialID get_material_ID(Renderer *r, int3 voxel) {
	return r->voxels[get_voxel_index(r, voxel)];
}

Material get_material(Renderer *r, MaterialID id) {
	return r->materials[id - 1];
}

//---- ray -------------------------------------------------------------------//

void init_voxel_traversal(Ray ray, int3 voxel, int3 *stepDir, float3 *tMax, float3 *tDelta) {
	*stepDir = (int3){
		(ray.direction.x >= 0) ? 1 : -1,
		(ray.direction.y >= 0) ? 1 : -1,
		(ray.direction.z >= 0) ? 1 : -1
	};

	*tMax = (float3){
		(ray.direction.x != 0) ? (voxel.x + stepDir->x - ray.origin.x) / ray.direction.x : MAXFLOAT,
		(ray.direction.y != 0) ? (voxel.y + stepDir->y - ray.origin.y) / ray.direction.y : MAXFLOAT,
		(ray.direction.z != 0) ? (voxel.z + stepDir->z - ray.origin.z) / ray.direction.z : MAXFLOAT
	};
	
	*tDelta = (float3){
		(ray.direction.x != 0) ? 1 / ray.direction.x * stepDir->x : MAXFLOAT,
		(ray.direction.y != 0) ? 1 / ray.direction.y * stepDir->y : MAXFLOAT,
		(ray.direction.z != 0) ? 1 / ray.direction.z * stepDir->z : MAXFLOAT
	};
}

void step_voxel_traversal(int3 *voxel, int3 stepDir, float3 *tMax, float3 tDelta, int *side) {
	if(tMax->x < tMax->y) {
		if(tMax->x < tMax->z) {
			voxel->x += stepDir.x;
			tMax->x += tDelta.x;
			*side = 0;
		} else {
			voxel->z += stepDir.z;
			tMax->z += tDelta.z;
			*side = 2;
		}

	} else {
		if(tMax->y < tMax->z) {
			voxel->y += stepDir.y;
			tMax->y += tDelta.y;
			*side = 1;
		} else {
			voxel->z += stepDir.z;
			tMax->z += tDelta.z;
			*side = 2;
		}
	}
}

void get_voxel_traversal_info(Ray ray, int3 voxel, int3 stepDir, int side, float3 *hitPos, int3 *normal) {
	switch(side) {
		case 0:
			hitPos->x = (float)voxel.x;
			hitPos->y = ray.origin.y + (hitPos->x - ray.origin.x) * ray.direction.y / ray.direction.x;
			hitPos->z = ray.origin.z + (hitPos->x - ray.origin.x) * ray.direction.z / ray.direction.x;
			*normal = (int3){-stepDir.x, 0, 0};
			break;
		
		case 1:
			hitPos->y = (float)voxel.y;
			hitPos->x = ray.origin.x + (hitPos->y - ray.origin.y) * ray.direction.x / ray.direction.y;
			hitPos->z = ray.origin.z + (hitPos->y - ray.origin.y) * ray.direction.z / ray.direction.y;
			*normal = (int3){0, -stepDir.y, 0};
			break;
		
		case 2:
			hitPos->z = (float)voxel.z;
			hitPos->y = ray.origin.y + (hitPos->z - ray.origin.z) * ray.direction.y / ray.direction.z;
			hitPos->x = ray.origin.x + (hitPos->z - ray.origin.z) * ray.direction.x / ray.direction.z;
			*normal = (int3){0, 0, -stepDir.z};
			break;

	}
}

bool cast_ray(Renderer *r, Ray ray, int3 *voxel, float3 *hitPos, int3 *normal, Material *material) {
	int3 stepDir;
	float3 tMax, tDelta;

	init_voxel_traversal(ray, *voxel, &stepDir, &tMax, &tDelta);
	
	int side;

	while(1) {
		step_voxel_traversal(voxel, stepDir, &tMax, tDelta, &side);

		if(is_out_of_scene(r, *voxel))
			return false;

		MaterialID id = get_material_ID(r, *voxel);

		if(id != 0) {
			*material = get_material(r, id);
			get_voxel_traversal_info(ray, *voxel, stepDir, side, hitPos, normal);
			return true;
		}
		
	}
}

float3 get_color(Renderer *r, Ray ray, int maxDepth) {
	// TODO: maxDepth
	// TODO: russian rulette
	// TODO: start with ray-box intersection check

	float3 mask = 1;
	float3 color = 0;
	int3 voxel = convert_int3(ray.origin);
	
	for(int i = 0; i < maxDepth; i++) {
		float3 hitPos;
		int3 iNormal;
		Material material;
		
		if(cast_ray(r, ray, &voxel, &hitPos, &iNormal, &material)) {
			float3 fNormal = convert_float3(iNormal);

			// return material.color;
			
			// TODO: dielectric material
			if(material.type == MATERIAL_TYPE_LIGHT_SOURCE) {
				color = mask * material.color * material.details.lightSource.brightness;
				break;

			} else if(material.type == MATERIAL_TYPE_LAMBERTIAN) {
				ray = (Ray){
					hitPos,
					normalize(fNormal + get_random_unit_vector(r->rng))
				};
				voxel += iNormal;
				mask *= material.color;
			
			} else if(material.type == MATERIAL_TYPE_METAL) {
				ray = (Ray){
					hitPos,
					normalize(get_reflection_dir(ray.direction, fNormal) + get_random_unit_vector(r->rng) * material.details.metal.fuzz)
				};
				voxel += iNormal;
				mask = mask * (1 - material.details.metal.tint) + mask * material.color * material.details.metal.tint;

			}

		} else {
			color = mask * r->bgColor;
			break;
		
		}

	}

	return color;
}

//---- main ------------------------------------------------------------------//

// TODO: better sampling
kernel void renderer(
	global float3 *image, int2 imageSize,
	global unsigned char *voxels, int3 sceneSize,
	global Material *materials,
	float3 bgColor,
	Camera camera,
	int sampleNumber,
	ulong seed
) {
	int id = get_global_id(0);
	ulong rng = init_rng_2(id, seed);

	Renderer r = (Renderer){
		image, imageSize,
		voxels, sceneSize,
		materials,
		bgColor,
		camera,
		&rng
	};

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

	float3 color = get_color(&r, ray, 100) * camera.exposure * camera.aperture;
	
	image[id] = (image[id] * sampleNumber + color) / (sampleNumber + 1);
	// image[id] = color;

}