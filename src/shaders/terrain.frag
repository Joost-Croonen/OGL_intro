#version 330
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;
in float Height;

uniform vec3 albedo;
uniform float occlusion;
uniform float roughness;
uniform float metalness;

uniform sampler2D heightMap;
uniform float heightScale;
uniform vec2 terrainSize; 

struct light {
	int type; // 0 = point, 1 = directional
	vec3 origin;
	vec3 color;
};

uniform light lights[16];
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

vec3 fresnelSchlick(float HdotV, vec3 F0)
{
	return F0 +(1-F0)*pow(clamp(1 - HdotV, 0.0, 1.0), 5.0);
}

float heightmapShadow(vec2 startCoords, vec3 lightDir)
{
    // lightDir must be normalized and point TOWARDS the sun/light source
    if (lightDir.y <= 0.0) return 0.0; // Sun below horizon

	const float shadowSoftness = 2.0;

    int numSteps = 64;
    float shadow = 1.0;
    vec2 currentPos = startCoords;
	float currentHeight = texture(heightMap, currentPos).r;
    float maxDistance = 1.0; 
    float stepSize = maxDistance / float(numSteps);
	vec3 rayStep = lightDir * stepSize;
	float deltaHeight = rayStep.y / heightScale;
    vec2 rayUV = rayStep.xz;

    for (int i = 0; i < numSteps; i++) {
        currentPos += rayUV;
        currentHeight += deltaHeight; 

        // Stop marching if the ray leaves the heightmap UV bounds [0, 1]
        //if (any(lessThan(currentPos, vec2(0.000001))) || any(greaterThan(currentPos, vec2(0.9999999))))
        //    break;

        float texHeight = texture(heightMap, currentPos).r;

        // Compare world height against world height directly
		float heightDiff = texHeight - currentHeight;
        if (heightDiff > 0.0) {
			float travelDist = float(i + 1) * stepSize;
			float penumbra = 1.0 - clamp(shadowSoftness * (heightDiff / travelDist), 0.0, 1.0);
            shadow = min(shadow, penumbra); 
            // Early exit if pixel is completely in shadow
            if (shadow <= 0.01) {
                shadow = 0.0;
                break;
            }
        }
    }
    return shadow;
}

vec3 heightColor(){
	vec3 color = vec3(0.0);
	float highFrequencyNoise = texture(heightMap, TexCoords*100 + vec2(0.23, 0.32)).r;
	float midFreauencyNoise = texture(heightMap, TexCoords*10 + vec2(0.43, 0.11)).r;
	float lowFrequencyNoise = texture(heightMap, TexCoords*1 + vec2(0.13, 0.31)).r;
	float redNoise = texture(heightMap, TexCoords*10 + vec2(0.73, 0.12)).r;
	float greenNoise = texture(heightMap, TexCoords*10 + vec2(0.93, 0.57)).r;
	float blueNoise = texture(heightMap, TexCoords*10 + vec2(0.02, 0.41)).r;
	float h = Height + highFrequencyNoise * 4 + 20 * midFreauencyNoise + lowFrequencyNoise * 50;
	float mixFactor = clamp((70 - h) / 10, 0.0, 1.0);
	vec3 snow = vec3(0.95, 0.95, 0.99);	//white 
	vec3 rock = vec3(0.2, 0.2, 0.25);	// dark grey
	rock +=  0.02 * vec3(redNoise, greenNoise, blueNoise);
	color = mix(snow, rock, mixFactor);
	return albedo;
}

void main()
{
	vec3 test = vec3(0.0);
	vec3 Albedo = heightColor();

	vec3 N = normalize(Normal);
	vec3 V = normalize(camPos - WorldPos);

	vec3 Lo = vec3(0.0);
	for (int i=0; i<numLights; i++)
	{
		vec3 L = vec3(0.0);
		if (lights[i].type == 0){ // point light
			vec3 position = lights[i].origin;
			L = normalize(position - WorldPos);
		}
		else if (lights[i].type == 1){ // directional light
			vec3 direction = lights[i].origin;
			L = normalize(-direction);
		}
		vec3 H = normalize(L + V);

		float NdotL = max(dot(N, L), 0.0);
		float NdotV = max(dot(N, V), 0.0);
		float HdotV = max(dot(H, V), 0.0);


		float attenuation	= 1.0;
		if (lights[i].type == 0){ // point light
			float dist			= length(lights[i].origin - WorldPos);
			attenuation	= 1.0 / (dist * dist);
		}
		vec3 radiance		= lights[i].color;

		vec3 F0 = mix(F0_base, Albedo, metalness);
		vec3 F = fresnelSchlick(HdotV, F0);

		float D = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(NdotL, NdotV, roughness);

		vec3 num = D * G * F;
		float denom = 4 * NdotV * NdotL + 0.0001;
		vec3 specular = num / denom;
		//test = 4 * specular;

		vec3 kS = F;
		vec3 kD = (vec3(1.0) - kS);
		kD *= (1.0 - metalness);

		vec3 diffuse = kD * Albedo / PI;
		
		float shadow = heightmapShadow(TexCoords, L);

		Lo += shadow * (diffuse + specular) * radiance * attenuation * NdotL;
		//test = vec3(NdotL, 10 * specular.x, 0.0);
		//test = vec3(pow(max(dot(N, H), 0.0), 16.0));
		//test = H;
	}
	
	vec3 ambient = vec3(0.01, 0.01, 0.03) * Albedo * occlusion;
	vec3 color = Lo + ambient;

	color = color / (color + vec3(1.0));		// tone mapping
	color = pow(color, vec3(1.0/2.2));			// gamma correction
	test = N;

	FragColor = vec4(color, 1.0);
	//FragColor = vec4(test, 1.0);
}