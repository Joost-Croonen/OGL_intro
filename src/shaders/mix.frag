#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float mixValue;

void main()
{
	vec3 tex1 = texture(texture1, TexCoords).xyz;
	vec3 tex2 = texture(texture2, TexCoords).xyz;
	vec3 color = mixValue * tex1 + (1.0 - mixValue) * tex2;
	FragColor = vec4(color, 1.0);

}