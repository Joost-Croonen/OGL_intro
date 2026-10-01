#version 330
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normals;
in vec2 TexCoords;
in vec3 Colors;
in vec3 GroundNormal;
in float vertHeight;
in float Wind;

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

const vec3 leafTint = vec3(0.060, 0.320, 0.030);//vec3(0.220, 0.502, 0.016);
const float directScatter = 3.0;
const float ambientScatter = 2.0;
const float GIstrength = 0.05;

const float sssDistortion = 0.25; 
const float sssPower      = 4.0;  // Focuses forward scattering glow tightness

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

vec2 beerLambertCanopyScattering(float height, float extinction, vec3 L){
	float depth = 1.0 - height;
	float k = extinction / max(L.y, 0.01);
	float m = 2.0 * extinction;
	const float omega = 0.5;
	float pathLenght = depth / max(L.y, 0.01);
	float transmitted = exp(-k * depth);
	float downScatter = (omega * k / max(m - k, 0.001)) * 
                           (exp(-k * depth) - exp(-m * depth));

	float upScatter   = (omega * k / (m + k)) * 
                           (exp(-k * depth) - exp(-k - m * (1.0 - depth)));

	float scattered = (downScatter + upScatter);
	return vec2(transmitted, scattered);
}

vec2 scatteringApprox(float height, float sigmaT, float omega, float diffusionLength, vec3 L){
	float depth = 1.0 - height;
	float pathLength = depth / max(L.y, 0.01);
	float opticalDepht = pathLength * sigmaT;
	float transmitted = exp(-opticalDepht);
	float scattered = omega * opticalDepht * transmitted;
	float scatter2 = omega * (1.0 - transmitted) * exp(-opticalDepht/diffusionLength);
	return vec2(transmitted, scatter2);
}

vec2 scatteringApprox2(float height, float directScatter, float ambientScatter, float GIstrength, vec3 L, vec3 Ns){
	float LdotNs = dot(L, Ns);
	if (LdotNs <= 0.0)
		return vec2(0.0);
	float d = 1.0 - height;
	float mu0 = max(LdotNs, 0.01);
	float tauDirect = directScatter / mu0;
	float tauAmbient = ambientScatter / mu0;
	float transmitted = exp(-d * tauDirect);
	float ambient = 0.0;
	if (abs(directScatter - ambientScatter) < 1e-3)
		ambient = d * transmitted;
	else{
		ambient = (exp(-d * tauDirect) - exp(-d * tauAmbient) ) / (tauAmbient - tauDirect);
	}
	float dPeak;
	if (abs(directScatter - ambientScatter) < 1e-3){
		dPeak = 1.0 / tauDirect;
	}
	else{
		dPeak =
			log(tauAmbient / tauDirect)
			/ (tauAmbient - tauDirect);
	}
	dPeak = clamp(dPeak, 0.0, 1.0);
	float giPeak;
	if (abs(directScatter - ambientScatter) < 1e-3){
		giPeak = dPeak * exp(-tauDirect * dPeak);
	}
	else {
		giPeak =
			(exp(-tauDirect * dPeak) - exp(-tauAmbient * dPeak))
			/ (tauAmbient - tauDirect);
	}
	float normalization = 1.0 / max(giPeak, 1e-5);
	ambient *= normalization * GIstrength * mu0;
	return vec2(transmitted, ambient);
}


void main()
{
	vec3 test = vec3(0.0);

	float h = clamp(vertHeight, 0, 1);

	vec3 N = normalize(Normals);
	if (!gl_FrontFacing) {
        N = -N;
    }
	vec3 V = normalize(camPos - WorldPos);
	vec3 U = normalize(GroundNormal);


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
		vec3 radiance = lights[i].color;

		vec3 F0 = mix(F0_base, Colors, metalness);
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

		vec3 diffuse = kD * Colors / PI;
		
		float shadow = 1.0;

		vec2 scattering = scatteringApprox2(h, directScatter, ambientScatter, GIstrength, L, U);
		float transmitted = scattering.x;
		vec3 inScattering = scattering.y * leafTint * radiance * attenuation;
		test = inScattering;
		//test = vec3(transmitted);
		//test = vec3(scattering.y);
		//test = inScattering;
		vec3 finalRadiance = shadow * radiance * attenuation * transmitted;
		Lo += (diffuse + specular) * finalRadiance * NdotL;
		Lo += inScattering;
		//test = vec3(NdotL, 10 * specular.x, 0.0);
		//test = vec3(pow(max(dot(N, H), 0.0), 16.0));
		//test = H;

		// Light vector pointing through the back of the leaf toward camera
		vec3 backLightDir = L + N * sssDistortion;
		float sssIntensity = max(0.0, dot(V, -normalize(backLightDir)));
		float sssFactor = pow(sssIntensity, sssPower);

		// Thickness mask: Tips are translucent, roots are opaque
		float bladeThicknessMask = smoothstep(0.1, 1.0, h);

		// Transmit tint (uses scatterAlbedo or a vibrant green/yellow transmission map)
		const vec3 transmitTint = leafTint;//vec3(0.080, 0.550, 0.060); 

		vec3 directTranslucency = sssFactor * bladeThicknessMask * transmitTint * finalRadiance;
		Lo += directTranslucency;
	}

	float occlusionStrenght = exp(- 1.66 * directScatter * (1.0-h));
	vec3 ambient = 0.1 * vec3(0.529, 0.808, 0.922) * Colors * occlusionStrenght;
	Lo += ambient;

	vec3 color = Lo;
	//color = color / (color + vec3(1.0));		// tone mapping
	//color = pow(color, vec3(1.0/2.2));			// gamma correction
	
	//test = N.yxz;
	//test = U * 0.5 + 0.5;
    //float dist = length(camPos - WorldPos);
    //float distBlend = smoothstep(0, 1, dist / 200.0);
	//test = vec3(distBlend);
	//test = vec3(occlusionStrenght);
	//test = Colors;
	test = vec3(Wind);
	FragColor = vec4(color, 1.0);
	
	//FragColor = vec4(test, 1.0);
}