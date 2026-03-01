#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;
uniform bool perspective;

//void main()
//{             
//    float depthValue = texture(depthMap, TexCoords).r;
//    // FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
//    FragColor = vec4(vec3(depthValue), 1.0); // orthographic
//}

void main()
{    
    //float depth = gl_FragCoord.z;
    float depth = texture(depthMap, TexCoords).r;
    float ndc = 2.0 * depth - 1.0;
    float near = 0.1;
    float far = 100.0;
    float linDepth = (2.0 * near * far)/(far + near - ndc * (far - near));
    if (perspective) FragColor = vec4(vec3(linDepth/far), 1.0);    //perspective
    else FragColor = vec4(vec3(texture(depthMap, TexCoords).r), 1.0);      //orhtographic
}