#version 330 core
out vec4 FragColor; 
in vec3 Normal;
in vec3 FragPos;
in vec2 texCoord;
//in vec3 ourColor;


//uniform sampler2D ourTexture1;
//uniform sampler2D ourTexture2;

struct Material{
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight{
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float cutOff;
    float outerCutOff; 

    float constant;
    float linear;
    float quadratic;
};

#define NR_POINT_LIGHTS 4 
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform vec3 viewPos;


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    // Directional light
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // Point light
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, viewDir);
    // Spot light
    result += CalcSpotLight(spotLight, norm, viewDir);
    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){
    // light direction
    vec3 lightDir = normalize(-light.direction);
    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, texCoord).rgb;
    // diffuse
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, texCoord).rgb;
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, texCoord).rgb;
    // total
    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir){
    // light direction
    vec3 lightDir = normalize(light.position - FragPos);
    // attentunation
    float dist          = length(light.position - FragPos);
    float attentunation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
    // ambient
    vec3 ambient = light.ambient * attentunation * texture(material.diffuse, texCoord).rgb;
    // diffuse
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = light.diffuse * attentunation * diff * texture(material.diffuse, texCoord).rgb;
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * attentunation * spec * texture(material.specular, texCoord).rgb;
    // total
    return ambient + diffuse + specular;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir){
    // light direction
    vec3 lightDir = normalize(light.position - FragPos);
    // distance attentunation
    float dist          = length(light.position - FragPos);
    float attentunation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
    // spot intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // ambient
    vec3 ambient = light.ambient * attentunation * texture(material.diffuse, texCoord).rgb;
    // diffuse
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = light.diffuse * attentunation * intensity * diff * texture(material.diffuse, texCoord).rgb;
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * attentunation * intensity * spec * texture(material.specular, texCoord).rgb;
    // total
    return ambient + diffuse + specular;
}