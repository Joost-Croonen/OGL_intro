#version 460 core

out vec4 FragColor;

uniform vec2 resolution;
uniform int scale; 

const vec2 v00 = vec2(0.0, 0.0);
const vec2 v10 = vec2(1.0, 0.0);
const vec2 v01 = vec2(0.0, 1.0);
const vec2 v11 = vec2(1.0, 1.0);

// this does not guarantee unit vectors on sphere
// noise hash function Inigo Quilez
vec2 hash2(vec2 p){
    p = vec2( dot(p,vec2(127.1,311.7)),
              dot(p,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

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

vec2 rand_direction(vec2 p){
    uint randi = hash(uvec2(p));
    float randf = uint_to_normalized_float(randi);
    float theta = 2*3.14159265358979323846*randf;
    return vec2(cos(theta), sin(theta));
}

float perlin(vec2 p){
	vec2 i = floor(p);
    vec2 f = fract(p);

    vec2 t = f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);

    float s00 = dot(rand_direction(i + v00), f-v00);
    float s10 = dot(rand_direction(i + v10), f-v10);
    float s01 = dot(rand_direction(i + v01), f-v01);
    float s11 = dot(rand_direction(i + v11), f-v11);

    return mix( mix(s00, s10, t.x), mix(s01, s11, t.x), t.y);
}

void main(){
    vec2 p = gl_FragCoord.xy / resolution.xy;
    p.x *= resolution.x/resolution.y;
    p *= scale;
    float noise = perlin(p);
    vec3 color = vec3(noise * 0.5 + 0.5);
    FragColor = vec4(color, 1.0);
}