#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 18) out;

in VS_OUT{
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
} gs_in[];

out vec3 fColor;  

const float MAGNITUDE = 0.4;

uniform mat4 projection;

void main()	
{
	for(int i = 0; i<3; i++){
		fColor = vec3(0.0, 0.0, 1.0);
		gl_Position = projection * gl_in[i].gl_Position;
		EmitVertex();
		gl_Position = projection * (gl_in[i].gl_Position + vec4(gs_in[i].normal, 0.0) * MAGNITUDE);
		EmitVertex();
		EndPrimitive();
		
		fColor = vec3(1.0, 0.0, 0.0);
		gl_Position = projection * gl_in[i].gl_Position;
		EmitVertex();
		gl_Position = projection * (gl_in[i].gl_Position + vec4(gs_in[i].tangent, 0.0) * MAGNITUDE);
		EmitVertex();
		EndPrimitive();
		
		fColor = vec3(0.0, 1.0, 0.0);
		gl_Position = projection * gl_in[i].gl_Position;
		EmitVertex();
		gl_Position = projection * (gl_in[i].gl_Position + vec4(gs_in[i].bitangent, 0.0) * MAGNITUDE);
		EmitVertex();
		EndPrimitive();
	}
}