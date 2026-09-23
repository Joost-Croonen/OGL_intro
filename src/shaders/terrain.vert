#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out float Height;
out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

uniform sampler2D heightMap;
uniform float time;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform float heightScale;
uniform vec2 terrainSize; 

void main()
{
    float scale = terrainSize.x * heightScale;
    TexCoords = mod(aTex + vec2(0.02 * time, 0.0), vec2(1.0));
    //vec2 sampleUV = aTex;// * ((terrainSize - 1.0) / terrainSize) + (0.5 / terrainSize);   
    float height = texture(heightMap, TexCoords).r * scale;
    vec2 grad = texture(heightMap, TexCoords).gb * heightScale * vec2(1.0, terrainSize.x/terrainSize.y);
    Normal = normalMatrix * normalize(vec3(-grad.x, 1.0, -grad.y));
    vec3 pos = vec3(aPos.x, height, aPos.z);
    vec4 worldPos = model * vec4(pos, 1.0);
    Height = worldPos.y;
    WorldPos = worldPos.xyz;
    gl_Position = projection * view * worldPos;
}