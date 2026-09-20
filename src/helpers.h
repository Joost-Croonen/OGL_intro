#pragma once

float random() {
	return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

float lerp(float a, float b, float t) {
	return a + t * (b - a);
}

float smoothstep(float t) {
	return t * t * (3.0f - 2.0f * t);
}

float smootherstep(float t) {
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float smoothlerp(float a, float b, float t) {
	return a + smoothstep(t) * (b - a);
}

float smootherlerp(float a, float b, float t) {
	return a + smootherstep(t) * (b - a);
}
