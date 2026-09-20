#version 410 core

layout (quads, fractional_even_spacing, ccw) in;

uniform sampler2D heightMap;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec2 TessTexCoord[];

out float Height;

//void main()
//{
//	// patch coords
//	float u = gl_TessCoord.x;
//	float v = gl_TessCoord.y;
//
//	// get the texture coordinates of the four corners of the patch
//	vec2 t00 = TessTexCoord[0];
//	vec2 t01 = TessTexCoord[1];
//	vec2 t10 = TessTexCoord[2];
//	vec2 t11 = TessTexCoord[3];
//
//	// bilinear interpolation of texture coordinates across the patch
//	vec2 t0 = (t01 - t00) * u + t00;
//	vec2 t1 = (t11 - t10) * u + t10;
//	vec2 texCoord = (t1 - t0) * v + t0;
//
//	// sample the height from the height map and scale and offset it to the desired range
//	Height = texture(heightMap, texCoord).r * 64.0 - 16.0;
//
//	// get the position of the four corners of the patch
//	vec4 p00 = gl_in[0].gl_Position;
//	vec4 p01 = gl_in[1].gl_Position;
//	vec4 p10 = gl_in[2].gl_Position;
//	vec4 p11 = gl_in[3].gl_Position;
//
//	// compute surface normal
//	vec4 edge1 = p01 - p00;
//	vec4 edge2 = p10 - p00;
//	vec4 normal = normalize(vec4(cross(edge1.xyz, edge2.xyz), 0));
//
//	// bilinear interpolation of position across the patch
//	vec4 p0 = (p01 - p00) * u + p00;
//	vec4 p1 = (p11 - p10) * u + p10;
//	vec4 position = (p1 - p0) * v + p0;
//
//	// displace along the normal
//	position += normal * Height;
//
//	gl_Position = projection * view * model * position;
//}

void main()	{
	float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // get texcoords and positions of the four corners of the patch
    vec2 t00 = TessTexCoord[0]; // BL
    vec2 t10 = TessTexCoord[1]; // BR
    vec2 t11 = TessTexCoord[2]; // TR
    vec2 t01 = TessTexCoord[3]; // TL

    vec4 p00 = gl_in[0].gl_Position; // BL
    vec4 p10 = gl_in[1].gl_Position; // BR
    vec4 p11 = gl_in[2].gl_Position; // TR
    vec4 p01 = gl_in[3].gl_Position; // TL

    // bilinear interpolation of texture coordinates and positions across the patch
    vec2 t0 = mix(t00, t10, u); // Bottom edge (BL -> BR)
    vec2 t1 = mix(t01, t11, u); // Top edge    (TL -> TR)
    vec2 texCoord = mix(t0, t1, v);

    vec4 p0 = mix(p00, p10, u);
    vec4 p1 = mix(p01, p11, u);
    vec4 position = mix(p0, p1, v);

    // sample the height from the height map and scale and offset it to the desired range
    Height = texture(heightMap, texCoord).r * 64.0 - 16.0;

    // --- NORMAL & WINDING ALIGNMENT ---
    vec3 uVec = (p10 - p00).xyz; // Pointing RIGHT (+X)
    vec3 vVec = (p01 - p00).xyz; // Pointing NORTH (-Z)

    // (+X) cross (-Z) = +Y (Points UP, preserves CCW winding)
    vec4 normal = normalize(vec4(cross(uVec, vVec), 0.0));

    position += normal * Height;

    gl_Position = projection * view * model * position;
}
