#version 460 core
#define PI 3.14159265358979323846
out vec4 FragColor;

in vec2 TexCoords;

const float rayleighHoverR = 8.0/6371.0;
const float mieScaleHoverR = 1.2/6371.0;
const float Re = 6371.0;

float integrateDensity(float h0, float sinAlpha, float scaleHeight)
{
	const float R = Re;
	const float H = scaleHeight;
	const int numSteps = 10000;

	float maxLength = 2 * sqrt(20*R*H + 100*H*H);
	float stepLength = maxLength / numSteps;
	float dt = stepLength;
	float t = 0.0;
	float result = 0.0;
	for (int i = 0; i<numSteps; i++)
	{
		t += dt;
		float ht = sqrt(t*t + R*R + 2.0*R*h0 + h0*h0 + 2*t*(R+h0)*sinAlpha) - R;
		result += exp(-ht / H) * dt;
		if (ht < 0.0) {
			return 1e6;
		}
	}
	return result;
}

float integrateDensityLayer(float h0, float sinAlpha, float layerBottom, float layerTop)
{
	const float R = Re;
	const float HT = layerTop;
	const float HB = layerBottom;
	const float HM = (HT + HB) * 0.5;
	const int numSteps = 10000;

	float maxLength = 2 * sqrt(20*R*HT + 100*HT*HT);
	float stepLength = maxLength / numSteps;
	float dt = stepLength;
	float t = 0.0;
	float result = 0.0;
	for (int i = 0; i<numSteps; i++)
	{
		t += dt;
		float ht = sqrt(t*t + R*R + 2.0*R*h0 + h0*h0 + 2*t*(R+h0)*sinAlpha) - R;
		float density = smoothstep(HB, HM, ht) * (1.0 - smoothstep(HM, HT, ht));
		result += density * dt;
		if (ht < 0.0) {
			return 1e6;
		}
	}
	return result;
}

void main()
{
	float sinAlpha = TexCoords.x * 2.0 - 1.0;
	float h0 = TexCoords.y * 80.0;
	float rayleigh = integrateDensity(h0, sinAlpha, 8.0);
	float mie = integrateDensity(h0, sinAlpha, 1.2);
	float ozone = integrateDensityLayer(h0, sinAlpha, 15.0, 35.0);
	FragColor = vec4(rayleigh, mie, ozone, 1.0);
}