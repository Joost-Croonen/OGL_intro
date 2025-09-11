#version 330 core
out vec4 FragColor; 
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

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
    vec3 diffuseColor = texture(material.texture_diffuse1, TexCoord).rgb;
    vec3 specularColor = texture(material.texture_specular1, TexCoord).rgb;
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
    //float gamma = 2.2;
    //color = vec4(pow(color.rgb, vec3(1.0/gamma)), 1.0);
    FragColor = color;
}