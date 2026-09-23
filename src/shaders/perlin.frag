#version 460 core

out vec4 FragColor;

uniform vec2 resolution;
uniform int frequency; 
uniform float amplitude;

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

vec3 perlin_gradient_tiling(vec2 uv, float F, int seed=0){
    vec2 p = uv * F;
    vec2 i = floor(p);
    vec2 f = fract(p);

    vec2 b = f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);
    vec2 db = 30.0 * f * f * (f * (f - 2.0) + 1.0);

    vec2 i00 = mod(i + v00, F);
    vec2 i10 = mod(i + v10, F);
    vec2 i01 = mod(i + v01, F);
    vec2 i11 = mod(i + v11, F);

    vec2 g00 = rand_direction(i00 + ivec2(seed, 1311*seed));
    vec2 g10 = rand_direction(i10 + ivec2(seed, 1311*seed));
    vec2 g01 = rand_direction(i01 + ivec2(seed, 1311*seed));
    vec2 g11 = rand_direction(i11 + ivec2(seed, 1311*seed));

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
    float dHdx = frequency * dHds * aspect;
    float dHdz = frequency * dHdt;

    return vec3(H, dHdx, dHdz);
}

void main(){
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    float aspect = resolution.x/resolution.y;
    uv.x *= aspect;
    vec3 noise = perlin_gradient_tiling(uv, frequency, 0);
    //vec3 scaled = amplitude * vec3(noise.x * 0.5 + 0.5, noise.y * 0.5, noise.z * 0.5);
    //FragColor = vec4(scaled, 1.0);
    FragColor = vec4(amplitude * noise, 1.0);
}