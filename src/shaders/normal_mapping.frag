#version 330 core

out vec4 FragColor; 

in vec3 FragPos;
in mat3 invTBN;
in mat3 TBN;
in vec3 Normal;
in vec2 TexCoords;
in vec3 tangentViewPos;
in vec3 tangentLightPos;
in vec3 tangentFragPos;

struct Material{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
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
uniform bool normal_mapping;

vec4 calc_light_TBN(int index)
{
    vec3 diffuseColor = texture(material.texture_diffuse1, TexCoords).rgb;
    vec3 specularColor = texture(material.texture_specular1, TexCoords).rgb;
    // attentunation
    float dist = length(light[index].position - FragPos);
    float attenuation = 1.0 / pow(dist, light[index].attenuation_power);
    // diffuse
    vec3 vert_normal = normalize(Normal);
    vec3 mapped_normal = texture(material.texture_normal1, TexCoords).rgb;
    mapped_normal = mapped_normal * 2.0 - 1.0;
    mapped_normal = normalize(TBN * mapped_normal);
    vec3 norm = normal_mapping ? mapped_normal : vert_normal;
    vec3 lightDir = normalize(light[index].position - FragPos);
    vec3 diffuse = max(dot(lightDir, norm), 0.0) * diffuseColor;
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfVec = normalize(lightDir + viewDir);
    vec3 specular = pow(max(dot(norm, halfVec), 0.0), material.shininess) * specularColor;

    return vec4(attenuation * (diffuse + specular) * light[index].color, 1.0);
}

vec4 calc_light_invTBN(int index)
{
    vec3 diffuseColor = texture(material.texture_diffuse1, TexCoords).rgb;
    vec3 specularColor = texture(material.texture_specular1, TexCoords).rgb;
    // attentunation
    float dist = length(light[index].position - FragPos);
    float attenuation = 1.0 / pow(dist, light[index].attenuation_power);
    // diffuse
    vec3 vert_normal = invTBN * normalize(Normal);
    vec3 mapped_normal = texture(material.texture_normal1, TexCoords).rgb;
    mapped_normal = normalize(mapped_normal * 2.0 - 1.0);
    vec3 norm = normal_mapping ? mapped_normal : vert_normal;
    vec3 lightDir = invTBN * normalize(light[index].position - FragPos);
    vec3 diffuse = max(dot(lightDir, norm), 0.0) * diffuseColor;
    // specular
    vec3 viewDir = invTBN * normalize(viewPos - FragPos);
    vec3 halfVec = normalize(lightDir + viewDir);
    vec3 specular = pow(max(dot(norm, halfVec), 0.0), material.shininess) * specularColor;

    return vec4(attenuation * (diffuse + specular) * light[index].color, 1.0);
}

vec4 calc_light_tangent(int index)
{
    vec3 diffuseColor = texture(material.texture_diffuse1, TexCoords).rgb;
    vec3 specularColor = texture(material.texture_specular1, TexCoords).rgb;
    // attentunation
    float dist = length(light[index].position - FragPos);
    float attenuation = 1.0 / pow(dist, light[index].attenuation_power);
    // diffuse
    vec3 vert_normal = invTBN * normalize(Normal);
    vec3 mapped_normal = texture(material.texture_normal1, TexCoords).rgb;
    mapped_normal = normalize(mapped_normal * 2.0 - 1.0);
    vec3 norm = normal_mapping ? mapped_normal : vert_normal;
    vec3 lightDir = normalize(tangentLightPos - tangentFragPos);
    vec3 diffuse = max(dot(lightDir, norm), 0.0) * diffuseColor;
    // specular
    vec3 viewDir = normalize(tangentViewPos - tangentFragPos);
    vec3 halfVec = normalize(lightDir + viewDir);
    vec3 specular = pow(max(dot(norm, halfVec), 0.0), material.shininess) * specularColor;

    return vec4(attenuation * (diffuse + specular) * light[index].color, 1.0);
}

void main()
{
    vec4 color = vec4(0);
    for(int i = 0; i < min(num_lights, MAX_LIGHTS); ++i)
        color += calc_light_tangent(i);
    //float gamma = 2.2;
    //color = vec4(pow(color.rgb, vec3(1.0/gamma)), 1.0);
    FragColor = color;
    vec3 mapped_normal = texture(material.texture_normal1, TexCoords).rgb;
    mapped_normal = normalize(mapped_normal * 2.0 - 1.0);
    //FragColor = vec4(mapped_normal, 1.0);
}
