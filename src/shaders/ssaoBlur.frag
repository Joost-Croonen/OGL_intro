#version 330 core
out float FragColor;
  
in vec2 TexCoords;

uniform sampler2D ssaoInput;
uniform int ssaoNoiseSize;

void main()
{
	vec2 texelSize = 1.0/vec2(textureSize(ssaoInput, 0));
	float result = 0.0;
	for (int i=-ssaoNoiseSize/2; i<ssaoNoiseSize/2; ++i)
	{
		for (int j=-ssaoNoiseSize/2; j<ssaoNoiseSize/2; ++j)
		{
			vec2 offset = vec2(float(i), float(j)) * texelSize;
			result += texture(ssaoInput, TexCoords + offset).r;
		}
	}
	FragColor = result / (4 * 4);
}

