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