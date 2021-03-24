//---- structs ---------------------------------------------------------------//

typedef uint MaterialID;

typedef struct Material {
	int type;
	float3 color;
	float tint;
	float fuzzyness;
	float refIdx;
} Material;

typedef struct Ray {
	float3 origin;
	float3 direction;
} Ray;

typedef struct Camera {
	float3 pos;
	float3 rot;
	float sensorWidth;
	float focalLength;
	float aperture;
	float exposure;
} Camera;

typedef struct Renderer {
	global float3 *image;
	int2 imageSize;
	global int *voxels;
	int3 sceneSize;
	global Material *materials;
	float3 bgColor;
	Camera camera;
	ulong *rng;

} Renderer;

//---- rng -------------------------------------------------------------------//

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

//---- math ------------------------------------------------------------------//

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

float3 random_unit_vector(ulong *rng) {
	float3 r;

	float cosTheta = rand_float_in_range(-1, 1, rng);
	float cosPhi = rand_float_in_range(-1, 1, rng);

	float sinTheta = sqrt(1 - cosTheta * cosTheta);
	float sinPhi = sqrt(1 - cosPhi * cosPhi);

	r.x = sinTheta * cosPhi;
	r.y = sinTheta * sinPhi;
	r.z = cosTheta;

	return r;
}

//---- ray -------------------------------------------------------------------//

int out_of_scene(Renderer *r, int3 pos) {
	return pos.x < 0 || pos.x >= r->sceneSize.x || pos.y < 0 || pos.y >= r->sceneSize.y || pos.z < 0 || pos.z >= r->sceneSize.z;
}

MaterialID get_material_ID(Renderer *r, int3 pos) {
	return r->voxels[pos.x + pos.y * r->sceneSize.x + pos.z * r->sceneSize.x * r->sceneSize.y];
}

Material get_material(Renderer *r, MaterialID id) {
	return r->materials[id - 1];
}

int cast_ray(Renderer *r, Ray ray, float3 *hitPos, float3 *normal, Material *material) {
	int3 pos = {
		(int)floor(ray.origin.x),
		(int)floor(ray.origin.y),
		(int)floor(ray.origin.z)
	};

	int3 step = {
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

		if(out_of_scene(r, pos)) {
			return 0;
		}

		MaterialID id = get_material_ID(r, pos);

		if(id == 0)
			continue;

		*material = get_material(r, id);

		if(side == 1) {
			hitPos->x = (float)pos.x;
			hitPos->y = ray.origin.y + (hitPos->x - ray.origin.x) * ray.direction.y / ray.direction.x;
			hitPos->z = ray.origin.z + (hitPos->x - ray.origin.x) * ray.direction.z / ray.direction.x;

			*normal = (float3){-step.x, 0, 0};
		} else if(side == 2) {
			hitPos->y = (float)pos.y;
			hitPos->x = ray.origin.x + (hitPos->y - ray.origin.y) * ray.direction.x / ray.direction.y;
			hitPos->z = ray.origin.z + (hitPos->y - ray.origin.y) * ray.direction.z / ray.direction.y;

			*normal = (float3){0, -step.y, 0};
		} else if(side == 3) {
			hitPos->z = (float)pos.z;
			hitPos->y = ray.origin.y + (hitPos->z - ray.origin.z) * ray.direction.y / ray.direction.z;
			hitPos->x = ray.origin.x + (hitPos->z - ray.origin.z) * ray.direction.x / ray.direction.z;

			*normal = (float3){0, 0, -step.z};
		}

		return 1;
		
	}
}

float3 get_color(Renderer *r, Ray ray) {

	// TODO: implement ambient color? (initial color value)
	float3 color = 0;
	float3 mask = 1;
	
	// TODO: implement maxDepth
	for(int i = 0; i < 10000; i++) {
		Material material;
		float3 normal;
		float3 hitPos;

		if(cast_ray(r, ray, &hitPos, &normal, &material)) {
			//return (float3){normal.x / 2 + 0.5, normal.y / 2 + 0.5, normal.z / 2 + 0.5};
			//return material.color;

			if(material.type == 0) {
				color = mask * material.color;
				break;

			} else if(material.type == 1) {
				float3 direction = normal + random_unit_vector(r->rng);
				ray = (Ray){hitPos, direction};
				mask *= material.color;

			} else if(material.type == 2) {
				float3 direction = reflection_dir(ray.direction, normal) + random_unit_vector(r->rng) * material.fuzzyness;
				ray = (Ray){hitPos, direction};
				mask = mask * (1 - material.tint) + mask * material.color * material.tint;
			}

			// TODO: dielectric material

		} else {
			color = mask * r->bgColor;
			break;
		}

	}

	return color;

}

//---- main ------------------------------------------------------------------//

kernel void main(global float3 *image, int2 imageSize, global int *voxels, int3 sceneSize, global Material *materials, float3 bgColor, Camera camera, int sampleNumber, ulong seed) {
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

	float3 color = get_color(&r, ray) * camera.exposure * camera.aperture;
	
	image[id] = (image[id] * sampleNumber + color) / (sampleNumber + 1);
}