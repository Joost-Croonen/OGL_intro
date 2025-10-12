#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform float gamma;
uniform float exposure;

const float offset = 1.0/360.0;
    

void sreenSampler(inout vec3 sreenSamples[9]) 
{
    vec2 offsets[9] = vec2[](
        vec2(-offset, offset),
        vec2(0.0,     offset),
        vec2(offset,  offset),
        vec2(-offset, 0.0),
        vec2(0.0,     0.0),
        vec2(offset,  0.0),
        vec2(-offset, -offset),
        vec2(0.0,     -offset),
        vec2(offset,  -offset)
    );
    for(int i = 0; i<9; i++){
        sreenSamples[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
}

vec4 invertColor(vec4 input_color)
{
    //vec4 screenColor = texture(screenTexture, TexCoords);
    return vec4(vec3(1.0-input_color), 1.0);
}

vec4 grayScaleColor(vec4 input_color)
{
    //vec4 screenColor = texture(screenTexture, TexCoords);
    float average = ( 0.2126 * input_color.r + 0.7152 * input_color.g + 0.0722 * input_color.b)/3.0;
    return vec4(average, average, average, 1.0);
}

vec4 reinhardToneMappingColor(vec4 input_color)
{
    vec3 mapped = input_color.rgb / (input_color.rgb + vec3(1.0));
    return vec4(mapped, 1.0);
}

vec4 exposureToneMappingColor(vec4 input_color)
{
    vec3 mapped = vec3(1.0) - exp(-input_color.rgb * exposure);
    return vec4(mapped, 1.0);
}

vec4 gammaColor(vec4 input_color)
{                                                                                                                                                                                   
    //vec4 screenColor = texture(screenTexture, TexCoords);
    return vec4(pow(input_color.rgb, vec3(1.0/gamma)), 1.0);
}

vec4 sharpenColor()
{
    float sharpKernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );
    vec3 sharpCol = vec3(0.0);
    vec3 sreenSamples[9];
    sreenSampler(sreenSamples);
    for(int i = 0; i<9; i++){
        sharpCol += sharpKernel[i] * sreenSamples[i];
    }
    return vec4(sharpCol, 1.0);
}

vec4 blurColor()
{
    float blurKernel[9] = float[](
        1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
        2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
        1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0
    );
    vec3 blurCol = vec3(0.0);
    vec3 sreenSamples[9];
    sreenSampler(sreenSamples);
    for(int i = 0; i<9; i++){
        blurCol += blurKernel[i] * sreenSamples[i];
    }
    return vec4(blurCol, 1.0);
}

vec4 edgeColor()
{
    float edgeKernel[9] = float[](
        1,  1,  1,
        1, -8,  1,
        1,  1,  1
    );
    vec3 edgeCol = vec3(0.0);
    vec3 sreenSamples[9];
    sreenSampler(sreenSamples);
    for(int i = 0; i<9; i++){
        edgeCol += edgeKernel[i] * sreenSamples[i];
    return vec4(edgeCol, 1.0);
    }
}

void main()
{
    FragColor = vec4(texture(screenTexture, TexCoords).rgb, 1.0);
    FragColor = exposureToneMappingColor(FragColor);
    FragColor = gammaColor(FragColor);
    //FragColor = grayScaleColor();
}