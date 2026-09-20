#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out float Height;

uniform sampler2D heightMap;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    float height = texture(heightMap, aTex).r * 256.0 - 64.0;
    vec3 pos = vec3(aPos.x, height, aPos.z);
    vec4 worldPos = model * vec4(pos, 1.0);
    Height = worldPos.y;
    gl_Position = projection * view * worldPos;
}