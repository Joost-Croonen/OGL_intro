#version 330 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0
layout (location = 1) in vec3 aNormal; // the normal variable has attribute position 1
layout (location = 2) in vec2 aTexCoord; // texture coordinates have attribute position 2
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;  

out vec3 FragPos;
out mat3 TBN;
out mat3 invTBN;
out vec3 Normal;
out vec2 TexCoords;
out vec3 tangentViewPos;
out vec3 tangentLightPos;
out vec3 tangentFragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
    vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
    T = normalize(T - dot(T, N) * N);   // Gramm-Schmidt renormalize
    B = cross(N, T);                    // Cross product to find new Bitangent
    TBN = mat3(T, B, N);
    invTBN = transpose(TBN);
    tangentViewPos = invTBN * viewPos;
    tangentLightPos = invTBN * lightPos;
    tangentFragPos = invTBN * FragPos;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}