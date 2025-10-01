#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Material{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};

struct PointLight{
    vec3 position;
    vec3 color;
    float attenuation_power;
};

#define MAX_LIGHTS 10 
uniform int num_lights;
uniform PointLight pointLights[MAX_LIGHTS];
uniform Material material;

uniform vec3 viewPos;
uniform float far_plane;

uniform samplerCube shadowCubeMap;

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);  

float calc_shadow(vec3 fragPos, vec3 lightPos)
{
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    float closestDepth = texture(shadowCubeMap, fragToLight).r * far_plane;
    float bias = 0.15; 
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = 0.001 + (viewDistance / far_plane) / 50.0 + (max(currentDepth - closestDepth, 0.0) / currentDepth) / 10.0;
    float shadow = 0.0;
    for(int i = 0; i < samples; ++i)
    {
        float pcfDepth = texture(shadowCubeMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r * far_plane;
        if(currentDepth - bias > pcfDepth) shadow += 1.0;
    }
    shadow /= samples;
    return shadow;
}

vec3 calc_point_light(int index)
{
    // attentunation
    float dist = length(pointLights[index].position - fs_in.FragPos);
    float attenuation = 1.0 / pow(dist, pointLights[index].attenuation_power);
    attenuation = 1.0;
    // diffuse
    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(pointLights[index].position - fs_in.FragPos);
    float diffuse_strength = max(dot(lightDir, norm), 0.0);
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfVec = normalize(lightDir + viewDir);
    float specular_strength = pow(max(dot(norm, halfVec), 0.0), material.shininess);
    // shadow
    float shadow = calc_shadow(fs_in.FragPos, pointLights[index].position);

    return vec3(attenuation * (diffuse_strength + specular_strength) * pointLights[index].color) * (1.0 - shadow);
}

void main()
{
    vec3 albedo = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 color = vec3(0);
    color += vec3(0.1, 0.1, 0.1); // ambient
    for(int i = 0; i < min(num_lights, MAX_LIGHTS); ++i)
        color += calc_point_light(i);
    vec3 fragToLight = fs_in.FragPos - pointLights[0].position;
    float closestDepth = texture(shadowCubeMap, fragToLight).r;
    FragColor = vec4(vec3(closestDepth), 1.0);
    FragColor = vec4(color * albedo, 1.0);
}
