#version 330 core
out vec4 FragColor;

void main()
{    
    float depth = gl_FragCoord.z;
    float ndc = 2.0 * depth - 1.0;
    float near = 0.1;
    float far = 100;
    float linDepth = (2.0 * near * far)/(far + near - ndc * (far - near));
    FragColor = vec4(vec3(linDepth/far), 1.0);
}