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

struct Light{
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

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
    // Light direction
    vec3 lightDir = normalize(light.position - FragPos);
    // vec3 lightDir = normalize(-light.direction);
    
    // Spot light calculations
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // Attenunation
    float dist          = length(light.position - FragPos);
    float attentunation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    // Ambient
    vec3 ambient = light.ambient * attentunation * texture(material.diffuse, texCoord).rgb;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuse = light.diffuse * attentunation * intensity * diff * texture(material.diffuse, texCoord).rgb;
    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * attentunation * intensity * spec * texture(material.specular, texCoord).rgb;

    FragColor = vec4(ambient + diffuse + specular, 1.0);

}