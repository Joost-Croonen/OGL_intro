#version 330 core
out vec4 FragColor; 
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;
uniform vec3 lightPos;

void main()
{
    vec3 texColor = texture(texture_diffuse1, TexCoord).rgb;
    // ambient
    vec3 ambient = texColor * 0.05;
    // diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 diffuse = max(dot(lightDir, norm), 0.0) * texColor;
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfVec = normalize(lightDir + viewDir);
    vec3 specular = pow(max(dot(halfVec, norm), 0.0), 32) * texColor;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}