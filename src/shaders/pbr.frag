#version 330
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D AlbedoMap;
uniform sampler2D NormalMap;
uniform sampler2D OcclusionRougnessMetalnessMap;

struct light {
	vec3 position;
	vec3 color;
};

const int NR_LIGHTS = 4;
uniform light lights[NR_LIGHTS];

uniform vec3 camPos;

const vec3 F0_base = vec3(0.04);
const float PI = 3.1415926535897932384626433832795;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = NdotH2 * (a2-1.0) +1;
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
	float num = NdotV;
	float denom = NdotV * (1-k) + k;

	return num / denom;
}

float GemoetrySmith(float NdotV, float NdotL, float roughness)
{	
	float r = roughness + 1.0;
	float k = (r*r) / 8.0;

	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float HdotV, vec3 F0)
{
	return F0 +(1-F0)*pow(1 - HdotV, 5);		// maybe add calmp although shouldnt be necessary
}

void main()
{
	vec3 albedo = texture(AlbedoMap, TexCoords).rgb;
	vec3 normal = texture(NormalMap, TexCoords).rgb;

	float occlusion = texture(OcclusionRougnessMetalnessMap, TexCoords).r;
	float roughness = texture(OcclusionRougnessMetalnessMap, TexCoords).g;
	float metalness = texture(OcclusionRougnessMetalnessMap, TexCoords).b;

	vec3 N = normalize(normal);
	vec3 V = camPos - WorldPos;

	vec3 Lo = vec3(0.0);
	for (int i=0; i<NR_LIGHTS; i++)
	{
		vec3 L = normalize(lights[i].position - WorldPos);
		vec3 H = normalize(L + V);

		float NdotL = max(dot(N, L), 0.0);
		float NdotV = max(dot(N, V), 0.0);
		float HdotV = max(dot(H, V), 0.0);

		float dist			= length(lights[i].position - WorldPos);
		float attenuation	= 1.0 / (dist * dist);
		vec3 radiance		= lights[i].color;

		vec3 F0 = mix(F0_base, albedo, metalness);
		vec3 F = fresnelSchlick(HdotV, F0);

		float D = DistributionGGX(N, H, roughness);
		float G = GemoetrySmith(NdotV, NdotL, roughness);

		vec3 num = D * G * F;
		float denom = 4 * NdotV * NdotL + 0.0001;
		vec3 specular = num / denom;

		vec3 kS = F;
		vec3 kD = (vec3(1.0) - kS) * (1.0 - metalness);

		vec3 diffuse = kD * albedo / PI;
		
		Lo += (diffuse + specular) * radiance * NdotL;   
	}
	vec3 ambient = vec3(0.03) * albedo * occlusion;
	vec3 color = Lo + ambient;

	color = color / (color + vec3(1.0));		// tone mapping
	color = pow(color, vec3(1.0/2.2));			// gamma correction

	FragColor = vec4(color, 1.0);
}