#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0/720.0;

void main()
{
    vec4 texcolor = texture(screenTexture, TexCoords);
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
    vec3 texSamples[9];
    for(int i = 0; i<9; i++){
        texSamples[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
    // inverted color
    vec4 inverted = vec4(vec3(1.0-texcolor), 1.0);
    // grayscale
    float average = ( 0.2126 * texcolor.r + 0.7152 * texcolor.g + 0.0722 * texcolor.b)/3.0;
    vec4 grayscale = vec4(average, average, average, 1.0);
    // sharpen
    float sharpKernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );
    vec3 sharpColor = vec3(0.0);
    for(int i = 0; i<9; i++){
        sharpColor += sharpKernel[i] * texSamples[i];
    }
    // blur
    float blurKernel[9] = float[](
        1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
        2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
        1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0
    );
    vec3 blurColor = vec3(0.0);
    for(int i = 0; i<9; i++){
        blurColor += blurKernel[i] * texSamples[i];
    }
    // edge
    float edgeKernel[9] = float[](
        1,  1,  1,
        1, -8,  1,
        1,  1,  1
    );
    vec3 edgeColor = vec3(0.0);
    for(int i = 0; i<9; i++){
        edgeColor += edgeKernel[i] * texSamples[i];
    }

    FragColor = vec4(edgeColor, 1.0);
}