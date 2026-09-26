#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec4 texcolor = texture(screenTexture, TexCoords);
    float color = texcolor.r;// * 0.5 + 0.5;
    FragColor = vec4(color, color, color, 1.0);
    if (texcolor.r > 1.0) //|| texcolor.g > 1.0 || texcolor.b > 1.0)
    {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
    if (texcolor.r < 0.0) //|| texcolor.g > 1.0 || texcolor.b > 1.0)
    {
        FragColor = vec4(0.0, 0.0, 1.0, 1.0);
    }
    FragColor = vec4(texcolor.rgb, 1.0);
}