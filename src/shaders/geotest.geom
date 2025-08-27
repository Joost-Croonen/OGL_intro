#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

in VS_OUT {
    vec3 color;
} gs_in[];

out vec3 fColor;

void main() {
    fColor = gs_in[0].color;
    vec4 position = gl_in[0].gl_Position;
    gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0); 
    EmitVertex();
    gl_Position = position + vec4( 0.2, -0.2, 0.0, 0.0); 
    EmitVertex();
    gl_Position = position + vec4(-0.2,  0.2, 0.0, 0.0); 
    EmitVertex();
    gl_Position = position + vec4( 0.2,  0.2, 0.0, 0.0); 
    EmitVertex();
    gl_Position = position + vec4( 0.0,  0.4, 0.0, 0.0); 
    fColor = vec3(1.0, 1.0, 1.0);
    EmitVertex();
    EndPrimitive();
}  