#version 330 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0

out vec3 ViewDir;
out float ViewH;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camPos; 
uniform vec3 planetPos;


void main()
{
    vec3 worldPos = vec3(model * vec4(aPos, 1.0));
    ViewDir = worldPos - camPos;
    ViewH = length(worldPos - planetPos);
    gl_Position = projection * view * vec4(worldPos, 1.0);
}