#version 460 core
#define PI 3.14159265358979323846
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out float Height;
out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

uniform vec2 resolution;
uniform int frequency; 
uniform float amplitude;
uniform float time;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform float heightScale;
uniform vec2 terrainSize; 

const vec2 v00 = vec2(0.0, 0.0);
const vec2 v10 = vec2(1.0, 0.0);
const vec2 v01 = vec2(0.0, 1.0);
const vec2 v11 = vec2(1.0, 1.0);

uint hash(uint x) {
    x ^= x >> 16;
    x *= 0x21F0AAADu;
    x ^= x >> 15;
    x *= 0xD35A2D97u;
    x ^= x >> 15;
    return x;
}

uint hash(uvec2 x){return hash(x.x + hash(x.y));}

float uint_to_normalized_float(uint x) {
    return uintBitsToFloat(x >> 9 | 0x3F800000u) - 1.0;
}

vec2 randPoint(vec2 p){
    uint randi1 = hash(uvec2(p + 1));
    uint randi2 = hash(randi1);
    float randf1 = uint_to_normalized_float(randi1);
    float randf2 = uint_to_normalized_float(randi2);
    return vec2(randf1, randf2);
}

vec2 rand_direction(vec2 p){
    uint randi = hash(uvec2(p));
    float randf = uint_to_normalized_float(randi);
    float theta = 2*3.14159265358979323846*randf;
    return vec2(cos(theta), sin(theta));
}

vec3 perlin_gradient(vec2 uv, float F){
    vec2 p = uv * F;
    vec2 i = floor(p);
    vec2 f = fract(p);

    vec2 b = f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);
    vec2 db = 30.0 * f * f * (f * (f - 2.0) + 1.0);

    vec2 g00 = rand_direction(i + v00);
    vec2 g10 = rand_direction(i + v10);
    vec2 g01 = rand_direction(i + v01);
    vec2 g11 = rand_direction(i + v11);

    float s00 = dot(g00, f-v00);
    float s10 = dot(g10, f-v10);
    float s01 = dot(g01, f-v01);
    float s11 = dot(g11, f-v11);

    float A = mix(s00, s10, b.s);
    float B = mix(s01, s11, b.s);

    float H = mix(A, B, b.t);

    float dAds = db.s * (s10 - s00) + mix(g00.x, g10.x, b.s);
    float dBds = db.s * (s11 - s01) + mix(g01.x, g11.x, b.s);
    float dHds = mix(dAds, dBds, b.t);

    float dAdt = mix(g00.t, g10.t, b.s);
    float dBdt = mix(g01.t, g11.t, b.s);
    float dHdt = db.t * (B - A) + mix(dAdt, dBdt, b.t);

    float aspect = resolution.x / resolution.y;
    float dHdx = F * dHds * aspect;
    float dHdz = F * dHdt;

    return vec3(H, dHdx, dHdz);
}

vec3 voronoi(vec2 uv, float F, int seed=0){
    vec2 p = uv * F;
    vec2 i = floor(p);
    vec2 f = fract(p);

    float minDist = 1.0;
    for (int x = -1; x<=1; x++){
        for (int y = -1; y<=1; y++){
            ivec2 n = ivec2(x, y);
                vec2 pt = randPoint(i + n);
                vec2 diff = (n + pt) - f;
                minDist = min(minDist, length(diff));
        }
    } 
    vec3 color = vec3(minDist);
    return color;
}

vec4 phacelle(vec2 uv, float F, vec2 dir, float offset, float normalization, int seed=0){
    vec2 p = uv * F;
    vec2 i = floor(p);
    vec2 f = fract(p);

    float weightSum = 0.0;
    vec2 phaseDir = vec2(0.0);
    vec2 sideDir = normalize(dir).yx + vec2(-1.0, 1.0) * F * 2 * PI;

    for (int x = -1; x<=1; x++){
        for (int y = -1; y<=1; y++){
            ivec2 n = ivec2(x, y);
                vec2 pt = randPoint(i + n) - 0.5;
                vec2 diff = f - (n + pt);
                float weight = max(0.0, exp(-dot(diff, diff) * 2.0) - 0.01111);
                weightSum += weight;
                float waveInput = dot(diff, sideDir) + offset;
                phaseDir += vec2(cos(waveInput), sin(waveInput)) * weight;
        }
    } 
    vec2 interp = phaseDir / weightSum;
    float mag = length(interp);
    mag = max(1.0 - normalization, mag);
    return vec4(interp / mag, sideDir);
}

void main()
{
    float scale = terrainSize.x * heightScale;
    TexCoords = mod(aTex + vec2(0.0 * time, 0.0), vec2(1.0));
    float height;
    vec2 grad;
    Normal = normalMatrix * normalize(vec3(-grad.x, 1.0, -grad.y));
    vec3 pos = vec3(aPos.x, height, aPos.z);
    vec4 worldPos = model * vec4(pos, 1.0);
    Height = worldPos.y;
    WorldPos = worldPos.xyz;
    gl_Position = projection * view * worldPos;
}