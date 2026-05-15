#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec4 texcolor = vec4(vec3(texture(screenTexture, TexCoords).r), 1.0);
    FragColor = texcolor;
}