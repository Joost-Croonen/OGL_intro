#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;

void main()
{
    vec4 texcolor = texture(image, TexCoords);
    float brightness = dot(texcolor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 2.0)
        FragColor = vec4(texcolor.rgb, 1.0);
	else
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}