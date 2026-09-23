#version 330 core
out vec4 FragColor;

in float Height;
uniform float heightScale;

void main()
{    
    float h = (Height + 0.5 * heightScale)/heightScale;
    FragColor = vec4(h, h, h, 1.0);
}