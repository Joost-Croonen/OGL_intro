#version 330 core
out vec4 FragColor; 

in VS_OUT{
    vec2 TexCoords;
    vec3 Normal;
    vec3 FragPos;
    vec4 FragPosLightSpace;
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

struct DirLight{
    vec3 direction;
    vec3 color;
};

#define MAX_LIGHTS 10 
uniform DirLight dirLight;
uniform PointLight pointLights[MAX_LIGHTS];
uniform vec3 viewPos;
uniform int num_lights;
uniform Material material;
uniform sampler2D shadowMap;

float calc_shadow(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal)
{
    vec3 project = (fragPosLightSpace.xyz/fragPosLightSpace.w) * 0.5 + 0.5;
    float currentDepth = project.z;
    if (currentDepth>1.0)
        return 0.0;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float closestDepth = texture(shadowMap, project.xy).r;
    float bias = max(0.015 * (1.0 - dot(lightDir, normal)), 0.0015);
    float closestShadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    float falloff = 0.25;
    for (int i=0; i<3; i++){
        for (int j=0; j<3; j++){
            float pcfDepth = texture(shadowMap, project.xy + vec2(i, j) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? mix(closestShadow, 1.0, min(abs(closestDepth - pcfDepth)/falloff, 1.0)) :mix(closestShadow, 0.0, min(abs(closestDepth - pcfDepth)/falloff, 1.0));
            //shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
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
    float shadow = calc_shadow(fs_in.FragPosLightSpace, lightDir, norm);

    return vec3(attenuation * (diffuse_strength + specular_strength) * pointLights[index].color) * (1.0 - shadow);
}

vec3 calc_dir_light()
{
    // diffuse
    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = normalize(dirLight.direction);
    float diffuse_strength = max(dot(lightDir, norm), 0.0);
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfVec = normalize(lightDir + viewDir);
    float specular_strength = pow(max(dot(norm, halfVec), 0.0), material.shininess);
    // shadow
    float shadow = calc_shadow(fs_in.FragPosLightSpace, lightDir, norm);

    return vec3((diffuse_strength + specular_strength) * dirLight.color) * (1.0 - shadow);
}

void main()
{
    vec3 albedo = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 color = vec3(0);
    color += vec3(0.1, 0.1, 0.1);
    for(int i = 0; i < min(num_lights, MAX_LIGHTS); ++i)
        color += calc_point_light(i);
    color += calc_dir_light();
    FragColor = vec4(color * albedo, 1.0);
}