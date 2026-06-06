#version 330 core
out vec4 FragColor;
in vec3 localPos;

uniform samplerCube environmentMap;
uniform float roughness;

const float PI = 3.14159265359;
const uint SAMPLE_COUNT = 4096u;

float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}  


vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;		//GGX is independent of phi so a uniform sample suffices
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y)); //inverse CDF method to find theta sample
	float sinTheta = sqrt(1 - cosTheta*cosTheta); // we only need costheta and sintheta so no need for arccos

	// Halfway vector in tangent space
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// Change to worldspace
	vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 T   = normalize(cross(up, N));
    vec3 B = cross(N, T);

	vec3 sampleVec = T * H.x + B * H.y + N * H.z;
	return sampleVec;
}


void main()
{	
	vec3 N = normalize(localPos);    
    vec3 R = N;
    vec3 V = R;

	vec3 result = vec3(0.0);
	float totalWeight = 0.0;
	for(uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);  
		vec3 H = ImportanceSampleGGX(Xi, N, roughness);
		vec3 L = normalize(2.0 * dot(V, H) * H - V); //reflect V(=N) around H to find L

		float NdotL = max(dot(N, L), 0.0);
		{
			// TODO: change miplevel based on pdf and roughness to reduce noise
			result += texture(environmentMap, L).rgb * NdotL;
			totalWeight += NdotL;
		}
	}
	result /= totalWeight;
	FragColor = vec4(result, 1.0);
	//FragColor = texture(environmentMap, localPos);
	//FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}