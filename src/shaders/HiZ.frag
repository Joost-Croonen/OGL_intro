#version 330 core

in vec2 TexCoords;
out float hiZ;

uniform sampler2D depth;
uniform int lod;

void main()
{
    if (lod == 0)
    {
        hiZ = texelFetch(depth, ivec2(gl_FragCoord.xy), 0).r;
    }
    else 
    {
        ivec2 dstCoord = ivec2(gl_FragCoord.xy);
        ivec2 srcCoord = dstCoord * 2;
        ivec2 srcSize = textureSize(depth, 0);
        ivec2 maxCoord = srcSize - ivec2(1);
        float d00 = texelFetch(depth, min(srcCoord + ivec2(0,0), maxCoord), 0).r;
        float d01 = texelFetch(depth, min(srcCoord + ivec2(0,1), maxCoord), 0).r;
        float d10 = texelFetch(depth, min(srcCoord + ivec2(1,0), maxCoord), 0).r;
        float d11 = texelFetch(depth, min(srcCoord + ivec2(1,1), maxCoord), 0).r;
        float res = min(min(d00, d01), min(d10, d11));

        //// If source width is odd and we are at the last column, include the 3rd column
        //if ((srcSize.x & 1) != 0 && dstCoord.x == (srcSize.x / 2) - 1) {
        //    float d20 = texelFetch(depth, min(srcCoord + ivec2(2,0), maxCoord), 0).r;
        //    float d21 = texelFetch(depth, min(srcCoord + ivec2(2,1), maxCoord), 0).r;
        //    res = min(res, min(d20, d21));
        //}
        //
        //// If source height is odd and we are at the last row, include the 3rd row
        //if ((srcSize.y & 1) != 0 && dstCoord.y == (srcSize.y / 2) - 1) {
        //    float d02 = texelFetch(depth, min(srcCoord + ivec2(0,2), maxCoord), 0).r;
        //    float d12 = texelFetch(depth, min(srcCoord + ivec2(1,2), maxCoord), 0).r;
        //    res = min(res, min(d02, d12));
        //}
        //
        //// If both are odd and we are at the bottom-right corner, include the extra corner pixel
        //if ((srcSize.x & 1) != 0 && (srcSize.y & 1) != 0 &&
        //    dstCoord.x == (srcSize.x / 2) - 1 && dstCoord.y == (srcSize.y / 2) - 1) {
        //    float d22 = texelFetch(depth, min(srcCoord + ivec2(2,2), maxCoord), 0).r;
        //    res = min(res, d22);
        //}

        hiZ = res;
    }
}  