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

    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform PointLight light;
uniform vec3 viewPos;

void main()
{
    vec3 diffuseColor = texture(material.texture_diffuse1, TexCoord).rgb;
    vec3 specularColor = texture(material.texture_specular1, TexCoord).rgb;
    // attentunation
    float dist = length(light.position - FragPos);
    float attentunation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    vec3 diffuse = max(dot(lightDir, norm), 0.0) * diffuseColor;
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfVec = normalize(lightDir + viewDir);
    vec3 specular = pow(max(dot(norm, halfVec), 0.0), material.shininess) * specularColor;
    FragColor = vec4(attentunation * (diffuse + specular), 1.0);
}