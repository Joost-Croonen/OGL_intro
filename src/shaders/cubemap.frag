#version 330 core
out vec4 FragColor;

in vec3 textureDir;

uniform samplerCube cubemap;
uniform float mipLevel;

void main()
{    
    FragColor = textureLod(cubemap, textureDir, mipLevel); 
}