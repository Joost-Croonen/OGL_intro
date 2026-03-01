#version 330 core
out vec4 FragColor;

float near = 0.1; 
float far  = 100.0; 
  
float LinearizeDepth(float depth) 
{
    // Convert back to NDC
    float z = depth * 2.0 - 1.0; 
    // Inverse the perspective projection
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{             
    // gl_FragCoord.z is the value currently in the depth buffer for this pixel
    float depth = LinearizeDepth(gl_FragCoord.z) / far; 
    FragColor = vec4(vec3(depth), 1.0);
}