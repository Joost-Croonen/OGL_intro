#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 ViewDir;

uniform sampler2D gDepth;
uniform sampler2D gNormal;
uniform sampler2D gColor;		

uniform float near;
uniform float far;

uniform mat4 inverseProjection;
uniform mat4 projection;
uniform mat3 normalView;

uniform int maxIter;
uniform int maxLod = 0;
const float maxDepth = 100.0;

struct ScreenRay
{
	vec2 start;
	vec2 dir;
	float invZStart;
	float invZDelta;
};

ScreenRay setupRay(vec3 viewPos, vec3 R){
	ScreenRay ray;
	vec3 viewStart = viewPos;// + N * 0.0001;
	vec4 clipStart = projection * vec4(viewStart, 1.0);
	ray.start = (clipStart.xy / clipStart.w) * 0.5 + 0.5;
	ray.invZStart = 1.0 / abs(viewStart.z);

	vec3 viewEnd = viewPos + R * maxDepth;
	vec4 clipEnd = projection * vec4(viewEnd, 1.0);
	vec2 rayEnd = (clipEnd.xy / clipEnd.w) * 0.5 + 0.5;
	float rayInvZEnd = 1.0 / abs(viewEnd.z);
	
	ray.dir = rayEnd - ray.start;
	ray.invZDelta = rayInvZEnd - ray.invZStart;
	return ray;
}

float rayDepth(ScreenRay ray, vec2 currentPos)
{
	vec2 dxy = (currentPos - ray.start);
	float t = dot(dxy, ray.dir) / dot(ray.dir, ray.dir);
	//t = clamp(t, 0.0, 1.0);

	float invZ = ray.invZStart + t * ray.invZDelta;
	float linDepth = abs(1.0 / invZ);
	return linDepth;
}

float textureDepth(vec2 tc, int lod, ScreenRay ray)
{
	float depth = textureLod(gDepth, tc + sign(ray.dir) * 1e-6, float(lod)).r;
    float ndc = 2.0 * depth - 1.0;
    float linDepth = abs((2.0 * near * far)/(far + near - ndc * (far - near)));
	return linDepth;
}


vec3 getViewPos()
{
	float depth = textureLod(gDepth, TexCoords, 0).r;
    float ndc = 2.0 * depth - 1.0;
	float viewZ = (2.0 * near * far) /
              (far + near - ndc * (far - near));
	return ViewDir * viewZ;
}

vec2 boundaryIntersect(int lod, ScreenRay ray, vec2 currentPos)
{
	ivec2 texRes = textureSize(gDepth, lod);
	float dx = 1.0 / texRes.x;
	float dy = 1.0 / texRes.y;
	vec2 nudgePos = currentPos + sign(ray.dir) * 1e-6;
	float xb = floor(nudgePos.x / dx);
	float yb = floor(nudgePos.y / dy);
	if (ray.dir.x > 0.0){xb += 1.0;}
	if (ray.dir.y > 0.0){yb += 1.0;}
	xb *= dx;
	yb *= dy;

	float t1 = (ray.dir.x != 0.0) ? (xb - ray.start.x) / ray.dir.x : 1e30;
    float t2 = (ray.dir.y != 0.0) ? (yb - ray.start.y) / ray.dir.y : 1e30;
	float t = min(t1, t2);

	return ray.start + t * ray.dir;
}


void main()
{
	vec3 V = normalize(ViewDir);
	vec3 N = normalize(normalView * texture(gNormal, TexCoords).rgb);
	vec3 R = normalize(reflect(V, N));
	
	vec3 viewPos = getViewPos(); 
	ScreenRay ray = setupRay(viewPos, R);
	
	int lod = maxLod;
	int iter = 0;
	bool hit = false;
	bool offscreen = false;

	vec2 currentPos = ray.start;
	currentPos = boundaryIntersect(0, ray, currentPos);
	float depth = rayDepth(ray, currentPos);
	float maxRayDepth = depth;

	if (R.z >= 0.0){
		offscreen = true;
		//FragColor = vec4(1.0, 0.0, 1.0, 1.0);		// Red: backwards ray
	}
	// Ray life
	while (!hit && !offscreen && (depth < maxDepth) && (iter < maxIter))
	{
		vec2 exitPos = boundaryIntersect(lod, ray, currentPos);
		// out of bounds checking
		bool outOfBounds = ((exitPos.x > 1.0) || (exitPos.y > 1.0) 
			|| (exitPos.x < 0.0) || (exitPos.y < 0.0));
		if (outOfBounds){
			if (lod == 0){
				offscreen = true;
				//FragColor = vec4(1.0, 0.0, 0.0, 1.0);		// Purple: out of bounds
				break;
			}
			else {
				lod -= 1;
				continue;
			}
		}
		maxRayDepth = rayDepth(ray, exitPos);
		vec2 samplePos = (currentPos + exitPos) * 0.5;
		float texDepth = textureDepth(samplePos, lod, ray);
		if ((maxRayDepth > texDepth) && (lod != 0))
		{
			// Refine search
			lod -= 1;
		}
		else if ((maxRayDepth > texDepth) && (lod == 0))
		{
			// Ray hit found
			hit = true;
			vec2 hitPos = currentPos;
			float minRayDepth = depth;
			if (minRayDepth < texDepth * 1.005){		//TODO: beter occlusion check
				FragColor = vec4(texture(gColor, hitPos).rgb, 1.0);
			}
			//else{FragColor = vec4(0.0, 0.0, 1.0, 1.0);}		// Blue: ray occlusion
			break;
		}
		else
		{
			// No hit on current lod
			// lod escalation
			while (lod < maxLod){			
				ivec2 parentRes = textureSize(gDepth, lod + 1);
				ivec2 oldCell = ivec2((currentPos + sign(ray.dir) * 1e-6) * parentRes);
				ivec2 newCell = ivec2((exitPos + sign(ray.dir) * 1e-6) * parentRes);
				if (oldCell != newCell){
					lod++;
				}
				else{
					break;
				}
			}
			// step forward
			currentPos = exitPos;
			depth = maxRayDepth;
		}
		iter++;
	}
	//if (iter == maxIter) {FragColor = vec4(0.0, 1.0, 1.0, 1.0);}  // Cyan: iteration time-out
}
