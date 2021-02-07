#include <stdio.h>
#include <stdlib.h>

int rand_int(int x, int y, int id) {
	int seed = x + id;
	int t = seed ^ (seed << 11);  
	int result = y ^ (y >> 19) ^ (t ^ (t >> 8));
	return result;
}

int main(void) {
	printf("%d\n", RAND_MAX);

	for(int i = 0; i < 10; i++) {
		int r = rand_int(rand(), rand(), i);
		float rf = (float)r / (float)RAND_MAX;
		printf("%d, %f\n", r, rf);
	}
}