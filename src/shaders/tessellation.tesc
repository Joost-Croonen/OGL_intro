#version 410 core
layout (vertices = 4) out;

in vec2 TexCoord[];
out vec2 TessTexCoord[];

uniform mat4 model;
uniform mat4 view;


void main()
{
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	TessTexCoord[gl_InvocationID] = TexCoord[gl_InvocationID];


	if (gl_InvocationID == 0)
    {
        // Step 1: define constants to control tessellation parameters
	    // set these as desired for your world scale
        const int MIN_TESS_LEVEL = 4;
        const int MAX_TESS_LEVEL = 64;
        const float MIN_DISTANCE = 2;
        const float MAX_DISTANCE = 2000;

        // Step 2: transform vertex position to view space
        vec4 viewPos00 = view * model * gl_in[0].gl_Position;   //BL
        vec4 viewPos10 = view * model * gl_in[1].gl_Position;   //BR
        vec4 viewPos11 = view * model * gl_in[2].gl_Position;   //TR
        vec4 viewPos01 = view * model * gl_in[3].gl_Position;   //TL

        // Step 3: compute the distance to camera
        float distance00 = clamp((abs(viewPos00.z)-MIN_DISTANCE)/(MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);
        float distance10 = clamp((abs(viewPos10.z)-MIN_DISTANCE)/(MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);
        float distance11 = clamp((abs(viewPos11.z)-MIN_DISTANCE)/(MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);
        float distance01 = clamp((abs(viewPos01.z)-MIN_DISTANCE)/(MAX_DISTANCE - MIN_DISTANCE), 0.0, 1.0);

        // Step 4: compute tessellation levels based on distance
        float tessLevel0 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance00, distance01));
        float tessLevel1 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance00, distance10));
        float tessLevel2 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance10, distance11));
        float tessLevel3 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance01, distance11));

        // Step 5: set tessellation levels for outer edges and inner edges
        gl_TessLevelOuter[0] = tessLevel0;
        gl_TessLevelOuter[1] = tessLevel1;
        gl_TessLevelOuter[2] = tessLevel2;
        gl_TessLevelOuter[3] = tessLevel3;

        gl_TessLevelInner[0] = max(tessLevel1, tessLevel3);
        gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
    }

	
}