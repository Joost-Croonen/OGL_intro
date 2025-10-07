#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;  

out VS_OUT{
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
} vs_out;

uniform mat4 view;
uniform mat4 model;

void main()
{
	gl_Position = view * model * vec4(aPos, 1.0);
	vs_out.normal = normalize(mat3(transpose(inverse(view * model))) * aNormal);
	vs_out.tangent = normalize(mat3(transpose(inverse(view * model))) * aTangent);
	vs_out.bitangent = normalize(mat3(transpose(inverse(view * model))) * aBitangent);
}