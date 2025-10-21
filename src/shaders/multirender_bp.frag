#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

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
uniform Material material;
uniform PointLight light[MAX_LIGHTS];
uniform vec3 viewPos;
uniform int num_lights;

vec4 calc_light(int index)
{
    vec3 diffuseColor = texture(material.texture_diffuse1, TexCoords).rgb;
    vec3 specularColor = texture(material.texture_specular1, TexCoords).rgb;
    // attentunation
    float dist = length(light[index].position - FragPos);
    float attenuation = 1.0 / pow(dist, light[index].attenuation_power);
    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light[index].position - FragPos);
    vec3 diffuse = max(dot(lightDir, norm), 0.0) * diffuseColor;
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfVec = normalize(lightDir + viewDir);
    vec3 specular = pow(max(dot(norm, halfVec), 0.0), material.shininess) * specularColor;

    return vec4(attenuation * (diffuse + specular) * light[index].color, 1.0);
}

void main()
{
    vec4 color = vec4(0);
    for(int i = 0; i < min(num_lights, MAX_LIGHTS); ++i)
        color += calc_light(i);
    FragColor = color;
    float brightness = dot(vec3(color), vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = color;
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}