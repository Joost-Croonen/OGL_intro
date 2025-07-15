#version 330 core
out vec4 FragColor; 
in vec3 Normal;
in vec3 FragPos;
in vec2 texCoords;

uniform vec3 viewPos;
uniform samplerCube skybox;

void main()
{
    float IOR = 1.0 / 1.52;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflDir = refract(-viewDir, norm, IOR);
    FragColor = vec4(texture(skybox, reflDir).rgb, 1.0);
}