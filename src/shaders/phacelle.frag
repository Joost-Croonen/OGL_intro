#version 460 core

#define PI 3.14159265358979323846

out vec4 FragColor;

uniform vec2 resolution;
uniform int frequency; 
uniform float amplitude;

unsigned int pcg_hash(unsigned int seed){
    unsigned int state = seed * 747796405u + 2891336453u;
    unsigned int word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

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

vec4 phacelle(vec2 uv, float F, vec2 dir, float freq, float offset, float normalization, int seed=0){
    vec2 p = uv * F;
    vec2 i = floor(p);
    vec2 f = fract(p);

    float weightSum = 0.0;
    vec2 phaseDir = vec2(0.0);
    vec2 sideDir = normalize(dir).yx * vec2(-1.0, 1.0) * freq * 2 * PI;
    offset += 2*PI;

    for (int x = -1; x<=2; x++){
        for (int y = -1; y<=2; y++){
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

void main(){
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    float aspect = resolution.x/resolution.y;
    uv.x *= aspect;
    vec2 dir = vec2(1.0, 0.0);
    vec4 noise = phacelle(uv, frequency, dir, 1.0, 0.5, 0);
    vec3 color = vec3(atan(noise.y, noise.x));
    FragColor = vec4(color, 1.0);
}