#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct int2 {
	int x;
	int y;
} int2;

typedef struct float2 {
	float x;
	float y;
} float2;


typedef struct Ray {
	float2 origin;
	float2 direction;
} Ray;

int main(void) {
	int2 mapSize = (int2){10, 10};
	const int map[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};

	Ray ray;
	ray.origin = (float2){4.5, 4.5};
	ray.direction = (float2){sqrt(2), sqrt(2)};

	int2 step;

	if(ray.direction.x > 0) step.x = 1;
	else if(ray.direction.x < 0) step.x = -1;
	else step.x = 0;

	if(ray.direction.y > 0) step.y = 1;
	else if(ray.direction.y < 0) step.y = -1;
	else step.y = 0;

	float2 tMax;
	if(ray.direction.x != 0) tMax.x = (ceil(ray.origin.x) - ray.origin.x) / ray.direction.x;
	else tMax.x = INFINITY;

	if(ray.direction.y != 0) tMax.y = (ceil(ray.origin.y) - ray.origin.y) / ray.direction.y;
	else tMax.y = INFINITY;

	printf("tMax: %f, %f\n", tMax.x, tMax.y);

	float2 tDelta;
	tDelta.x = 1 / ray.direction.x;
	tDelta.y = 1 / ray.direction.y;

	int hit = 0;
	int2 pos = (int2){(int)ray.origin.x, (int)ray.origin.y};
	float2 hitPos;
	while(1) {
		int side;

		if(tMax.x < tMax.y) {
			tMax.x += tDelta.x;
			pos.x += step.x;
			side = 1;
		} else {
			tMax.y += tDelta.y;
			pos.y += step.y;
			side = 2;
		}

		if(pos.x < 0 || pos.x > mapSize.x || pos.y < 0 || pos.y > mapSize.y)
			break;
		
		if(map[pos.x + pos.y * mapSize.x] != 0) {
			hit = 1;

			if(side == 1) {
				printf("side: x\n");
				hitPos.x = pos.x;
				hitPos.y = ray.origin.y + (hitPos.x - ray.origin.x) * ray.direction.y / ray.direction.x;
			} else if(side == 2) {
				printf("side: y\n");
				hitPos.y = pos.y;
				hitPos.x = ray.origin.x + (hitPos.y - ray.origin.y) * ray.direction.x / ray.direction.y;
			}
				
			break;
		}
	}

	if(hit) {
		float distance = sqrt(pow(hitPos.x - ray.origin.x, 2) + pow(hitPos.y - ray.origin.y, 2));
		
		printf("tMax': %f, %f\n", tMax.x, tMax.y);
		printf("tDelta: %f, %f\n", tDelta.x, tDelta.y);
		printf("pos: %d, %d\n", pos.x, pos.y);
		printf("hitPos: %f, %f\n", hitPos.x, hitPos.y);
		printf("distance: %f\n", distance);
		
	} else {
		printf("out of bounds\n");
	}
}
