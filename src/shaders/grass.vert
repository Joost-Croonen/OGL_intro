#version 330 core
#define PI 3.14159265358979323846

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normals;
out vec3 WorldPos;
out vec3 Colors;
out float vertHeight;
out vec3 GroundNormal;
out float Wind;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float heightScale;
uniform vec2 terrainSize;
uniform sampler2D heightMap;

uniform float chunkSize;
uniform vec2 chunkPos;

uniform vec3 camPos;
uniform float time;
uniform bool toggle;
uniform vec3 test;

const vec3 UP = vec3(0.0, 1.0, 0.0);
const vec3 FRONT = vec3(1.0, 0.0, 0.0);


uint hash(uint x) {
    x ^= x >> 16;
    x *= 0x21F0AAADu;
    x ^= x >> 15;
    x *= 0xD35A2D97u;
    x ^= x >> 15;
    return x;
}

uint hash(uvec2 v) {return hash(v.x + hash(v.y));}

float uint_to_normalized_float(uint x) {
    return uintBitsToFloat(x >> 9 | 0x3F800000u) - 1.0;
}

float rand1(int seed){
    uint i1 = hash(uint(seed));
    float f1 = uint_to_normalized_float(i1);
    return f1;
}

vec2 rand2(int seed){
    uint i1 = hash(uint(seed));
    uint i2 = hash(i1);
    float f1 = uint_to_normalized_float(i1);
    float f2 = uint_to_normalized_float(i2);
    return vec2(f1, f2);
}

// Translation Matrix
mat4 translate(vec3 t) {
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(t,   1.0)
    );
}

// Uniform Scale Matrix
mat4 scale(vec3 s) {
    return mat4(
        vec4(s.x, 0.0, 0.0, 0.0),
        vec4(0.0, s.y, 0.0, 0.0),
        vec4(0.0, 0.0, s.z, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
}

// Axis-Angle Rotation Matrix
mat4 rotate(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(
        vec4(oc * axis.x * axis.x + c,          oc * axis.x * axis.y + axis.z * s, oc * axis.z * axis.x - axis.y * s, 0.0),
        vec4(oc * axis.x * axis.y - axis.z * s, oc * axis.y * axis.y + c,          oc * axis.y * axis.z + axis.x * s, 0.0),
        vec4(oc * axis.z * axis.x + axis.y * s, oc * axis.y * axis.z - axis.x * s, oc * axis.z * axis.z + c,          0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
}

float easeOut(float t){
    return t * (1.0 - t*t);
}

//float smoothstep(float t) {
//	return t * t * (3.0f - 2.0f * t);
//}

vec2 map_to_terrain(vec2 uv){
    return (uv - 0.5) * terrainSize;
}

vec2 map_to_terrain_chunk(vec2 uv){
    return uv * chunkSize + chunkPos;
}

vec2 map_to_uv_chunk(vec2 uv){
    return (uv * chunkSize / terrainSize + chunkPos / terrainSize) + 0.5;
}

vec2 world_to_uv(vec2 worldPos){
    return worldPos / terrainSize + 0.5;
}

void main()
{
    //setup
    int seed = gl_InstanceID + int(hash(uvec2(chunkPos+1024)));//int(chunkPos.x) + int(hash(uint(chunkPos.y)));

    // position
    vec2 randVec2 = rand2(seed + 87);
    vec2 randUV = map_to_uv_chunk(randVec2);
    float height = texture(heightMap, randUV).r * heightScale; 
    vec2 offset = map_to_terrain_chunk(randVec2);
    vec3 position = vec3(offset.x, height, offset.y);
    float dist = length(camPos - position);

    // vertex merging
    vec3 mergePos = aPos;
    if (aPos.y > 0.1 && aPos.y <0.9){
        float mergeFactor = smoothstep(200, 300, dist);
        mergePos.y = mix(aPos.y, 0.5, mergeFactor);
        if (aPos.z > 0.01)
            mergePos.z = mix(aPos.z, 0.025, mergeFactor);
        else if (aPos.z < -0.01)
            mergePos.z = mix(aPos.z, -0.025, mergeFactor);
    }
    float h = mergePos.y;       // h goes from 0 to 1

    // wind
    vec3 windDir = vec3(1.0, 0.0, 0.0);
    vec2 windUV = randUV;
    windUV *= 10;
    windUV += .25 + 0.1 * time;
    float noise = texture(heightMap, windUV).r * 0.5 + 0.5;

    // bending
    float bendStrenght = 1.2 * noise * (rand1(seed) * 0.1 + 0.9);
    float theta = h * bendStrenght;
    mat4 bendMat = rotate(vec3(0.0, 0.0, 1.0), theta);
    vec3 bendPos = (bendMat * vec4(mergePos, 1.0)).xyz;

    // translate
    mat4 transMat = translate(position);

    // scale
    float scale_value = 5.0 * (1.0 + 0.1 * (rand1(seed + 107)*2.0-1.0));
    mat4 scaleMat = scale(vec3(scale_value));

    // rotate
    vec3 V = normalize(camPos - position);
    float randAngle = (rand1(seed + 223)*2.0-1.0) * 2.0 * PI;
    vec3 dir = vec3(cos(randAngle), 0.0, sin(randAngle));
    dir = normalize(windDir + 0.2 * dir);       // orient along winddir
    float viewDotDir = dot(V, dir);
    float dirBias = easeOut(viewDotDir);        // bias towards camera for thickening
    if (toggle) dir += dirBias * V;
    float angle = -atan(dir.z, dir.x);
    mat4 rotMat = rotate(UP, angle);

    // normals
    const float grassWidth = 0.05;
    const float normalCurveStrenght = 0.5;
    vec3 n0 = normalize(vec3(1.0, 0.0, aPos.z/grassWidth*normalCurveStrenght));  // init curved normal

    float cosT = cos(theta);
    float sinT = sin(theta);
    float dThetaOverH = theta; 

    vec3 bendNormal = vec3(
        n0.x * (cosT - dThetaOverH * sinT),
        n0.x * (sinT + dThetaOverH * cosT),
        n0.z
    );
    Normals = normalize(mat3(rotMat) * bendNormal);  // compensate for blade bending
    
    // terrain normal blending
    vec2 grad = texture(heightMap, randUV).gb * heightScale / terrainSize;
    GroundNormal = normalize(vec3(-grad.x, 1.0, -grad.y));
    float distBlend = smoothstep(50.0, 300.0, dist)* 0.7;
    Normals = normalize(mix(Normals, GroundNormal, distBlend));

    // color
    const vec3 color_tip = vec3(0.15, 0.30, 0.03);//vec3(0.282, 0.435, 0.220);
    const vec3 color_base = vec3(0.012, 0.04, 0.004);//0.2 * vec3(0.220, 0.502, 0.016);
    const float colorExponent = 3.0;
    float colorRamp = pow(h, colorExponent);
    vec3 color = mix(color_base, color_tip, colorRamp);
    
    // transform
    mat4 instanceModel = transMat * rotMat * scaleMat;
    vec4 pos = instanceModel * vec4(bendPos, 1.0);
    
    // output 
    TexCoords = aTexCoords;    
    WorldPos = pos.xyz;
    Colors = color;
    vertHeight = h;
    Wind = noise;
    gl_Position = projection * view * pos;
}