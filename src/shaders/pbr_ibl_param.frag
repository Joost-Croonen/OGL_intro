#version 330
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 albedo;
uniform float occlusion;
uniform float roughness;
uniform float metalness;

uniform samplerCube diffuseEnvMap;
uniform samplerCube specularEnvMap;
uniform sampler2D brdfLUT;

struct light {
	vec3 position;
	vec3 color;
};

uniform light lights[4];
uniform int numLights;

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
	float denom = NdotH2 * (a2-1.0) + 1.0;
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(float NdotL, float NdotV, float roughness)
{	
	float r = roughness + 1.0;
	float k = (r*r) / 8.0;

	float ggx1 = GeometrySchlickGGX(NdotV, k);
	float ggx2 = GeometrySchlickGGX(NdotL, k);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

void main()
{
	vec3 test = vec3(0.0);

	vec3 N = normalize(Normal);
	vec3 V = normalize(camPos - WorldPos);
	float NdotV = max(dot(N, V), 0.0);

	vec3 Lo = vec3(0.0);
	
	float r = roughness; // * roughness;
	vec3 F0 = mix(F0_base, albedo, metalness);

	// Lightsource contribution
	for (int i=0; i<numLights; i++)
	{
		vec3 L = normalize(lights[i].position - WorldPos);
		vec3 H = normalize(L + V);

		float NdotL = max(dot(N, L), 0.0);
		float HdotV = max(dot(H, V), 0.0);

		float dist			= length(lights[i].position - WorldPos);
		float attenuation	= 1.0 / (dist * dist);
		vec3 radiance		= lights[i].color * attenuation;

		vec3 F = fresnelSchlick(HdotV, F0);
		float D = DistributionGGX(N, H, r);
		float G = GeometrySmith(NdotL, NdotV, r);

		vec3 num = D * G * F;
		float denom = 4 * NdotV * NdotL + 0.0001;
		vec3 specular = num / denom;

		vec3 kS = F;
		vec3 kD = (vec3(1.0) - kS);
		kD *= (1.0 - metalness);

		vec3 diffuse = kD * albedo / PI;
		
		Lo += (diffuse + specular) * radiance * NdotL;   
	}
	// Environment contribution
	// Specular environment
	vec3 R = reflect(-V, N);  
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 specIrradiance = textureLod(specularEnvMap, R, r * MAX_REFLECTION_LOD).rgb;

	vec2 split_brdf = texture(brdfLUT, vec2(NdotV, r)).rg;
	vec3 brdf = F0 * split_brdf.x + split_brdf.y;
	vec3 specular = specIrradiance * brdf;

	// Diffuse environment
	vec3 F = fresnelSchlickRoughness(NdotV, F0, r);
	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= (1.0 - metalness);
	vec3 irradiance = texture(diffuseEnvMap, N).rgb;
	vec3 diffuse = kD * irradiance * albedo;

	// Combine environment lighting
	vec3 ambient = (diffuse + specular) * occlusion;
	// Final gather
	vec3 color = Lo;
	color += ambient;

	FragColor = vec4(color, 1.0);
	//FragColor = vec4(test, 1.0);
}