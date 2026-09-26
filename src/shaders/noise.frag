#version 330 core

#define PI 3.14159265358979323846

out vec4 FragColor;

uniform vec2 resolution;
uniform int frequency; 
uniform float amplitude;
uniform int octaves;
uniform float time;

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

vec3 perlin_gradient(vec2 uv, float F, int seed=0){
    vec2 p = uv * F;
    vec2 i = floor(p);
    vec2 f = fract(p);

    vec2 b = f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);
    vec2 db = 30.0 * f * f * (f * (f - 2.0) + 1.0);

    vec2 g00 = rand_direction(i + v00 + ivec2(seed, 1311*seed));
    vec2 g10 = rand_direction(i + v10 + ivec2(seed, 1311*seed));
    vec2 g01 = rand_direction(i + v01 + ivec2(seed, 1311*seed));
    vec2 g11 = rand_direction(i + v11 + ivec2(seed, 1311*seed));

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

vec3 fractalPerlin(vec2 p, float baseFrequency, int octaves, float lacunarity, float gain){
    vec3 result = vec3(0.0);
    float freq = baseFrequency;
    float amp = 1.0;
    for (int o = 0; o<octaves; ++o){
        result +=  amp * perlin_gradient(p, freq, 11);
        freq *= lacunarity;
        amp *= gain;
    }
    return result;
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

vec4 phacelle(vec2 uv, float F, vec2 dir, float freq, float offset, float normalization, int seed=0){
    vec2 p = uv * F;
    vec2 i = floor(p);
    vec2 f = fract(p);

    float weightSum = 0.0;
    vec2 phaseDir = vec2(0.0);
    vec2 sideDir = normalize(dir).yx * vec2(-1.0, 1.0) * freq * 2 * PI;
    offset += 2 * PI;

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
    return vec4(interp / mag, -F*sideDir);
}

float pow_inv(float t, float power) {
    // Flip, raise to the specified power, and flip back.
    return 1.0 - pow(1.0 - clamp(t, 0, 1), power);
}

float ease_out(float t) {
    // Flip by subtracting from one.
    float v = 1.0 - clamp(t, 0, 1);
    // Raise to a power of two and flip back.
    return 1.0 - v * v;
}

float smooth_start(float t, float smoothing) {
    if (t >= smoothing)
        return t - 0.5 * smoothing;
    return 0.5 * t * t / smoothing;
}

void main()
{
    float assumedSlope = 0.7;
    float slopeLength = 0.1;
    float cellScale = 0.7;
    float erosionScale = 0.15;
    float erosion_strength = 0.22;
    float erosoinDetail = 1.5;
    float gullyWeight = 0.5;
    float ridgeRounding = 0.1;
    float creaseRounding = 0.0;
    float onset = 1.25;
    float magnitude = 0.0;
    float rounding = 1.0;
    //float initRounding = 3.0 / freq;

    float freq = 12;
    float lacunarity = 2.0;
    float amp = 0.1;
    float gain = 0.5;

    vec3 test = vec3(0.0);
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    float aspect = resolution.x/resolution.y;
    uv.x *= aspect;
    uv += time * 0.0;
    vec3 baseNoise = fractalPerlin(uv, 3.0, 3, 2.0, 0.1);
    vec3 heightMap = baseNoise;
    float fadeTarget = clamp(heightMap.x / 0.6, -1.0, 1.0);
    float mask = ease_out(length(heightMap.yz/5.0));
    //float mask = ease_out(1.0 - abs(fadeTarget));
    float maskedTarget = (1.0 - mask) * fadeTarget;
    //float mask = pow(1.0 - abs(heightMap.x), 0.1);
    //test = vec3(1.0 - mask);
    //float roundingForInput = mix(creaseRounding, ridgeRounding, clamp(fadeTarget + 0.5, 0.0, 1.0)) * initRounding;
    //float combiMask = ease_out(smooth_start(slopeLength * onset, roundingForInput * onset));
    //heightMap *= 0.5;
    //heightMap += vec3(0.5, 0.0, 0.0);
    //test = vec3(fadeTarget);
    vec2 dir = heightMap.yz; // / slopeLength * assumedSlope;
    float result = 0.0;
    for (int o = 0; o<octaves; ++o){
        vec4 phacelleNoise = phacelle(uv, freq, normalize(dir), 0.7, 0.25, 0.5);
        //test = vec3(phacelleNoise.x, dir.x*0.5+0.5, dir.y*0.5+0.5);
        //float sloping = abs(phacelleNoise.y);
        dir += sign(phacelleNoise.y) * phacelleNoise.zw * amp * 0.5;
        //dir = mix(dir, sign(phacelleNoise.y) * phacelleNoise.zw, amp);

        vec3 gullies = vec3(phacelleNoise.x, phacelleNoise.y*phacelleNoise.zw);
        maskedTarget = (1.0 - mask) * fadeTarget;
        vec3 fadedGullies = mix(vec3(1.0 * fadeTarget, 0.0, 0.0), gullies * 0.5, mask);
        
        heightMap += fadedGullies * amp;
        fadeTarget = fadedGullies.x;

        //float newMask = ease_out(mix(0.0, 1.0, length( phacelleNoise.y / phacelleNoise.zw) - 0.5));
        //float newMask = 1.0 - clamp(abs(gullies.x * 0.5), 0.0, 1.0);
        //float newMask = mix(0.0, 1.0, 1.0 - abs(clamp(gullies.x, -1.0, 1.0)));
        //float newMask = ease_out(pow(1.0 - abs(gullies.x), 0.3));
        float newMask = ease_out(abs(phacelleNoise.y));
        mask = mask * newMask; 

        //float roundingForOctave = mix(creaseRounding, ridgeRounding, clamp(phacelleNoise.x + 0.5, 0.0, 1.0)) * rounding;
        //float newMask = ease_out(smooth_start(slopeLength * onset, roundingForOctave * onset));
        //combiMask = pow_inv(combiMask, erosoinDetail) * newMask;
    
        magnitude *= erosion_strength;
        freq *= lacunarity;
        rounding *= lacunarity;
        amp *= gain;
        //test = vec3(0.0, 0.0, phacelleNoise.x);
        //test += vec3(1.0 - newMask, 0.0, 0.0);
        //test = vec3(0.0, gullies.x, 0.0);
        //test = vec3(0.0, fadedGullies.x, 0.0);
        //test = vec3(phacelleNoise.x);
    }
    //test = vec3(baseNoise.x + 0.5);
    //test = vec3(fadedGullies.x);
    //test = vec3(combiMask);
    //test = vec3(baseNoise.x);
    //test = vec3(fadedGullies.x);
    //test = vec3(1.0 - mask);
    test = vec3(0.0, 0.0, fadeTarget * 0.5 + 0.5);
    //test = vec3(fadeTarget * 0.5 + 0.5);
    test = vec3((maskedTarget * 0.5 + 0.5)) + vec3(0.0, 0.2, 0.5);
    //test += vec3(1.0 - mask, 0.0, 0.0);
    //test = vec3(1.0 - mask);
    //test = vec3(fadeTarget > 0.8, 0.0, fadeTarget < -0.8);
    //test = vec3(heightMap.x * 0.5 + 0.5);
    FragColor = vec4(heightMap, 1.0);
    //FragColor = vec4(test, 1.0);
}