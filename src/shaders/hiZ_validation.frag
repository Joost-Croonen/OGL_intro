#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D hiZ;
uniform int lod;

void main()
{
	//ivec2 mipSize = textureSize(hiZ, lod);
	//ivec2 parentCoord = ivec2(gl_FragCoord.xy) * mipSize;
	//
	//float d = texelFetch(hiZ, parentCoord, lod).r;
	
	float near = 0.1;
	float far = 100.0;
	float d = textureLod(hiZ, TexCoords, lod).r;

	if (lod == 0)
	{
		float ndc = 2.0 * d - 1.0;
		float linDepth = (2.0 * near * far)/(far + near - ndc * (far - near));
		FragColor = vec4(vec3(linDepth/far), 1.0);
		return;
	}
	ivec2 parentSize = textureSize(hiZ, lod);
	ivec2 parentCoord =min(ivec2(TexCoords * vec2(parentSize)), parentSize - 1);
	ivec2 childBase = parentCoord * 2;
	ivec2 childSize = textureSize(hiZ, lod - 1);

    ivec2 c00 = min(childBase + ivec2(0,0), childSize - 1);
    ivec2 c01 = min(childBase + ivec2(1,0), childSize - 1);
    ivec2 c10 = min(childBase + ivec2(0,1), childSize - 1);
    ivec2 c11 = min(childBase + ivec2(1,1), childSize - 1);

    float d00 = texelFetch(hiZ, c00, lod - 1).r;
    float d01 = texelFetch(hiZ, c01, lod - 1).r;
    float d10 = texelFetch(hiZ, c10, lod - 1).r;
    float d11 = texelFetch(hiZ, c11, lod - 1).r;

	bool validate = d == min(min(d00, d01), min(d10, d11));
	if (validate){
		float ndc = 2.0 * d - 1.0;
		float linDepth = (2.0 * near * far)/(far + near - ndc * (far - near));
		FragColor = vec4(vec3(linDepth/far), 1.0);
	}
	else {
		FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
}