#version 330 core
out vec4 FragColor; 
in vec3 Normal;
in vec3 FragPos;
in vec2 texCoords;

uniform vec3 viewPos;
uniform samplerCube skybox;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflDir = reflect(-viewDir, norm);
    FragColor = vec4(texture(skybox, reflDir).rgb, 1.0);
}