#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssaoColorBuffer;

struct light {
	vec3 position;
	vec3 color;
	float attenuation;
};

const int NR_LIGHTS = 32;
uniform light lights[NR_LIGHTS];
uniform vec3 viewPos;
uniform bool ssaoFlag;

void main()	
{
	// retrieve from gbuffer
	vec3 FragPos = texture(gPosition, TexCoords).rgb;			//view
	vec3 Normal = texture(gNormal, TexCoords).rgb;				//view
	vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;			
	float Specular = texture(gAlbedoSpec, TexCoords).a;
	float ambient_occlusion = 1.0;
	if(ssaoFlag) ambient_occlusion = texture(ssaoColorBuffer, TexCoords).r;
	// lighting
	vec3 norm = normalize(Normal);								//view
	vec3 viewDir = normalize(- FragPos);						//view  !viewPos = (0,0,0)!
	vec3 lighting = Albedo * ambient_occlusion * 0.1;		
	for(int i = 0; i < NR_LIGHTS; ++i)
	{
		// attentunation
		float dist = length(lights[i].position - FragPos);		// light is now in viewpos!!
		float attenuation = 1.0 / pow(dist, lights[i].attenuation);
		// Diffuse
		vec3 lightDir = normalize(lights[i].position - FragPos);
		vec3 diffuse = max(dot(lightDir, norm), 0.0) * Albedo * lights[i].color;
		// Specular
		vec3 halfVec = normalize(lightDir + viewDir);
		vec3 specular = pow(max(dot(norm, halfVec), 0.0), 16.0) * Specular * lights[i].color;
		lighting += attenuation * (diffuse + specular);
	}
	FragColor = vec4(lighting, 1.0);
}