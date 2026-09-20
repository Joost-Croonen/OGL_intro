#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec4 texcolor = texture(screenTexture, TexCoords);
    FragColor = texcolor;
    if (texcolor.r > 1.0 || texcolor.g > 1.0 || texcolor.b > 1.0)
    {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
}