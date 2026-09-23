#version 460 core

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

void main(){
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    float aspect = resolution.x/resolution.y;
    uv.x *= aspect;
    vec3 noise = voronoi(uv, frequency, 0);
    FragColor = vec4(amplitude * noise, 1.0);
}