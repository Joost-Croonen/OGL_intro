#version 330 core
layout (location = 0) in vec3 aPos;   // the position variable has attribute position 0
layout (location = 1) in vec3 aNormal; // the normal variable has attribute position 1
layout (location = 2) in vec2 aTexCoord; // texture coordinates have attribute position 2
//layout (location = 3) in vec3 aColor; // the color variable has attribute position 3

//out vec3 ourColor; // output a color to the fragment shader
out vec2 texCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    //ourColor = aColor; // set ourColor to the input color we got from the vertex data
    texCoord = aTexCoord;
}