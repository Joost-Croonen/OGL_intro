#version 330
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D AlbedoMap;
uniform sampler2D NormalMap;
uniform sampler2D ORMMap;

uniform samplerCube irradianceMap;

struct light {
	vec3 position;
	vec3 color;
};

uniform light lights[4];
uniform int numLights;

uniform bool toggle;
uniform int caseNr;

uniform vec3 camPos;

const vec3 F0_base = vec3(0.04);
const float PI = 3.1415926535897932384626433832795;

vec3 worldNormalGradientTrick(vec3 tangentNormalMap)
{
	vec3 tangentNormal = tangentNormalMap * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

vec3 worldNormalGrammSchmidt(vec3 tangentNormalMap)
{
    vec3 tangentNormal = tangentNormalMap * 2.0 - 1.0;

    vec3 N = normalize(Normal);
    vec3 someVec = vec3(1.0, 0.0, 0.0);
	float dd = dot(someVec, N);
	vec3 T = vec3(0.0, 1.0, 0.0);;
	if (1.0-abs(dd) > 1e-6) {
		T = normalize(cross(someVec, N));
	}
    vec3 B = cross(N, T);

    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}

vec3 worldNormal(vec3 tangentNormalMap)
{
    // 1. Unpack the normal map (Now safe because sRGB is fixed!)
    vec3 tangentNormal = tangentNormalMap * 2.0 - 1.0;

    // 2. Get screen-space variations of position and UVs
    vec3 p_dx = dFdx(WorldPos);
    vec3 p_dy = dFdy(WorldPos);
    vec2 tc_dx = dFdx(TexCoords);
    vec2 tc_dy = dFdy(TexCoords);

    // 3. Compute the raw, UV-aligned Tangent and Bitangent (from Method 1)
    vec3 N = normalize(Normal);
    vec3 T = normalize(p_dx * tc_dy.t - p_dy * tc_dx.t);
    vec3 B = normalize(p_dy * tc_dx.s - p_dx * tc_dy.s);

    // 4. THE REAL GRAM-SCHMIDT ORTHOGONALIZATION
    // This forces T to be exactly 90 degrees to N, curing the flat-shading artifact
    T = normalize(T - dot(T, N) * N);
    
    // Determine the handedness of the coordinate system
    if (dot(cross(N, T), B) < 0.0) {
        T = T * -1.0;
    }

    // Now B can be safely calculated as a perfect 90-degree cross product
    B = normalize(cross(N, T));

    // 5. Build a perfectly square, UV-aligned matrix
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

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

	vec3 albedo = texture(AlbedoMap, TexCoords).rgb;
	vec3 normal = texture(NormalMap, TexCoords).rgb;
	
	float occlusion = texture(ORMMap, TexCoords).r;
	float roughness = texture(ORMMap, TexCoords).g;
	float metalness = texture(ORMMap, TexCoords).b;

	
	//vec3 N = vec3(0.0);
	//vec3 Ngt = worldNormalGradientTrick(normal);
	//vec3 Ngs = worldNormalGrammSchmidt(normal);
	//vec3 Nf = worldNormal(normal);
	//if (caseNr == 0){
	//	N = Nf;
	//}
	//else if (caseNr == 1){
	//	N = Ngt;
	//}
	//else if (caseNr == 2){
	//	N = Ngs;
	//}
	vec3 N = worldNormal(normal);
	vec3 V = normalize(camPos - WorldPos);
	float NdotV = max(dot(N, V), 0.0);

	vec3 Lo = vec3(0.0);
	
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
		float D = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(NdotL, NdotV, roughness);

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
	vec3 kS = fresnelSchlickRoughness(NdotV, F0, roughness);
	vec3 kD = 1.0 - kS;
	kD *= (1.0 - metalness);
	vec3 irradiance = texture(irradianceMap, N).rgb;
	vec3 diffuse = kD * irradiance * albedo;
	vec3 ambient = diffuse * occlusion;
	vec3 color = Lo;
	//ambient = vec3(0.03) * albedo;
	color += ambient;

	//test = mix(color, vec3(1.0), fresnelSchlickRoughness(NdotV, F0, roughness));
	//test = texture(AlbedoMap, TexCoords).rgb;
	//vec3 Ngt = worldNormalGradientTrick(normal)*0.5 + 0.5;
	//vec3 Ngs = worldNormalGrammSchmidt(normal)*0.5 + 0.5;
	//vec3 Nf = worldNormal(normal)*0.5 + 0.5;
	//if (caseNr == 0){
	//	test = Nf;
	//}
	//else if (caseNr == 1){
	//	test = Ngt;
	//}
	//else if (caseNr == 2){
	//	test = Ngs;
	//}
	//test = toggle ? Ngs: Nf;
	FragColor = vec4(color, 1.0);
	//FragColor = vec4(test, 1.0);
}