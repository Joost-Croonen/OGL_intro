#version 330 core
out float FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D ssaoNoise;

uniform vec3 samples[64];
uniform mat4 projection;

uniform int kernelSize;
uniform float radius;
uniform float bias;
uniform float occlusionStrength;

// tile noise texture over screen, based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(2560.0/4.0, 1440.0/4.0); 

void main()
{
	 vec3 fragPos = texture(gPosition, TexCoords).xyz;		//viewspace
	 vec3 normal = texture(gNormal, TexCoords).rgb;			//viewspace
	 vec3 randVec = texture(ssaoNoise, TexCoords * noiseScale).xyz;	//tangent space

	 vec3 tangent = normalize(randVec - normal * dot(randVec, normal));
	 vec3 bitangent = cross(normal, tangent);
	 mat3 TBN = mat3(tangent, bitangent, normal);			// tranform between tangent and view space

	 float occlusion = 0.0;

	 for (int i = 0; i<kernelSize; i++){
		vec3 tangentSamplePos = samples[i];					//tangent space
		vec3 samplePos = fragPos + TBN * tangentSamplePos * radius; // view space
		vec4 offset = projection * vec4(samplePos, 1.0);	// clip space
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;				// 0. to 1.
		float sampleDepth = texture(gPosition, offset.xy).z;
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(sampleDepth - fragPos.z));
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
	 }
	 FragColor = 1.0 - occlusion/kernelSize;
	 FragColor = pow(FragColor, occlusionStrength);
}