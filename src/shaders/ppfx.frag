#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0/360.0;
    

void texSampler(inout vec3 texSamples[9]) 
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
        texSamples[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
}

vec4 invertColor()
{
    vec4 texColor = texture(screenTexture, TexCoords);
    return vec4(vec3(1.0-texColor), 1.0);
}

vec4 grayScaleColor()
{
    vec4 texColor = texture(screenTexture, TexCoords);
    float average = ( 0.2126 * texColor.r + 0.7152 * texColor.g + 0.0722 * texColor.b)/3.0;
    return vec4(average, average, average, 1.0);
}

vec4 gammaColor()
{
    vec4 texColor = texture(screenTexture, TexCoords);
    float gamma = 2.2;
    return vec4(pow(texColor.rgb, vec3(1.0/gamma)), 1.0);
}

vec4 sharpenColor()
{
    float sharpKernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );
    vec3 sharpCol = vec3(0.0);
    vec3 texSamples[9];
    texSampler(texSamples);
    for(int i = 0; i<9; i++){
        sharpCol += sharpKernel[i] * texSamples[i];
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
    vec3 texSamples[9];
    texSampler(texSamples);
    for(int i = 0; i<9; i++){
        blurCol += blurKernel[i] * texSamples[i];
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
    vec3 texSamples[9];
    texSampler(texSamples);
    for(int i = 0; i<9; i++){
        edgeCol += edgeKernel[i] * texSamples[i];
    return vec4(edgeCol, 1.0);
    }
}

void main()
{
    FragColor = vec4(texture(screenTexture, TexCoords).rgb, 1.0);
}