#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 ViewDir;

uniform mat4 inverseProjection;

void main()
{ 
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aTexCoords;
    vec4 farPlaneRay = inverseProjection * vec4(aPos.xy, 1.0, 1.0);
    ViewDir = farPlaneRay.xyz / farPlaneRay.w;
    ViewDir /= -ViewDir.z;
}