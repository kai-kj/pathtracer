#define EPSILSON 0.00001f

typedef struct Ray {
	float3 origin;
	float3 direction;
} Ray;

typedef struct SceneInfo {
	float3 bgColor;
	int sphereCount;
} SceneInfo;

typedef struct Camera {
	float3 pos;
	float3 rot;
	int2 resolution;
	int useDOF;
	float sensorWidth;
	float focalLength;
	float aperture;
	float exposure;
	int samples;
	int maxRayDepth;
} Camera;

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

float3 rotate_vector(float3 vec, float3 rot) {
	vec.x = vec.x * cos(rot.z) - vec.y * sin(rot.z);
    vec.y = vec.x * sin(rot.z) + vec.y * cos(rot.z);

    vec.x = vec.x * cos(rot.y) + vec.z * sin(rot.y);
    vec.z = -vec.x * sin(rot.y) + vec.z * cos(rot.y);

    vec.y = vec.y * cos(rot.x) - vec.z * sin(rot.x);
    vec.z = vec.y * sin(rot.x) + vec.z * cos(rot.x);

	return vec;
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