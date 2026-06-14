#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 ViewDir;

uniform sampler2D gDepth;
uniform sampler2D gNormal;
uniform sampler2D gColor;		

uniform float near;
uniform float far;

uniform mat4 projection;
uniform mat3 normalView;

uniform int maxIter;
uniform int maxLod = 0;
const float maxDepth = 100.0;

struct ScreenRay
{
	vec2 start;
	vec2 dir;
	vec2 invDir;
	float invZStart;
	float invZDelta;
	ivec2 next;
};

ScreenRay setupRay(vec3 viewPos, vec3 R){
	ScreenRay ray;
	ivec2 texSize = textureSize(gDepth, 0);
	vec3 viewStart = viewPos;
	vec4 clipStart = projection * vec4(viewStart, 1.0);
	vec2 uvStart = (clipStart.xy / clipStart.w) * 0.5 + 0.5;
	ray.start = uvStart * texSize;
	ray.invZStart = 1.0 / abs(viewStart.z);

	vec3 viewEnd = viewPos + R * maxDepth;
	vec4 clipEnd = projection * vec4(viewEnd, 1.0);
	vec2 uvEnd = (clipEnd.xy / clipEnd.w) * 0.5 + 0.5;
	vec2 rayEnd = uvEnd * texSize;
	float rayInvZEnd = 1.0 / abs(viewEnd.z);
	
	ray.dir = rayEnd - ray.start;
	ray.invZDelta = rayInvZEnd - ray.invZStart;

	ray.next.x = ray.dir.x >= 0.0 ? 1: -1;
	ray.next.y = ray.dir.y >= 0.0 ? 1: -1;
	float invdirx = ray.dir.x!=0 ? 1.0 / ray.dir.x: 1e30;
	float invdiry = ray.dir.y!=0 ? 1.0 / ray.dir.y: 1e30;
	ray.invDir = vec2(invdirx, invdiry);
	return ray;
}

float rayDepth(ScreenRay ray, float t)
{
	float invZ = ray.invZStart + t * ray.invZDelta;
	float linDepth = abs(1.0 / invZ);
	return linDepth;
}

float textureDepth(ivec2 pxl, int lod)
{
	float depth = texelFetch(gDepth, pxl, lod).r;
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

void main()
{
	int lod = 0;
	int iter = 0;

	vec3 V = normalize(ViewDir);
	vec3 N = normalize(normalView * texture(gNormal, TexCoords).rgb);
	vec3 R = normalize(reflect(V, N));

	if (any(isnan(R))){
		FragColor = vec4(0.2, 0.2, 0.2, 1.0);		// Grey: No geometry
		return;
	}

	if (R.z >= 0.0){
		FragColor = vec4(1.0, 0.0, 0.0, 1.0);		// Red: backwards ray
		return;
	}

	vec3 viewPos = getViewPos(); 
	ScreenRay ray = setupRay(viewPos, R);
	ivec2 texSize = textureSize(gDepth, 0);


	//skip first pixel  
	// Todo: put this in ray init and make ray.start an ivec and make ray.org the float vec
	// in theory the viewrays go to the center of the pixels, and thus we could simplify this logic a lot
	ivec2 currentPix = ivec2(ray.start);
	ivec2 boundaries = currentPix + max(ray.next, ivec2(0));
	//boundaries = boundaries << lod;
	float tx = (boundaries.x - ray.start.x) * ray.invDir.x;
	float ty = (boundaries.y - ray.start.y) * ray.invDir.y;
	float t = min(tx, ty);
	bool LR = tx < ty;
	bool UD = !LR;
	currentPix += ray.next * ivec2(LR, UD);
	
	// Ray life
	while (iter < maxIter && t<1.0 )
	{
		++iter;
		float t_old = t;
		ivec2 boundaries = currentPix + max(ray.next, ivec2(0));
		boundaries = boundaries << lod;
		tx = (boundaries.x - ray.start.x) * ray.invDir.x;
		ty = (boundaries.y - ray.start.y) * ray.invDir.y;
		t = min(tx, ty);
		float rayDepth = rayDepth(ray, t);
		float texDepth = textureDepth(currentPix, lod);
		if (rayDepth < texDepth) {
			// step
			bool LR = tx < ty;
			bool UD = !LR;
			ivec2 nextPix = currentPix + ray.next * ivec2(LR, UD);
			// out of bounds check
			bool outOfBounds = (nextPix.x < 0 || nextPix.y < 0 
							 || nextPix.x >= (texSize.x>>lod) || nextPix.y >= (texSize.y>>lod));
			if (outOfBounds){
				if (lod==0){
					FragColor = vec4(1.0, 0.0, 1.0, 1.0);		// Purple: out of bounds
					return;
				}
				else {
					t = t_old;
					--lod;
					currentPix = currentPix << 1;
					bool shiftRight = (ray.start.x + t_old * ray.dir.x) >= float((currentPix.x + 1) << lod);
					bool shiftUp = (ray.start.y + t_old * ray.dir.y) >= float((currentPix.y + 1) << lod);
					currentPix += ivec2(shiftRight, shiftUp);
					continue;
				}
			}
			// lod escalation
			while (lod < maxLod){
				ivec2 parentNextPix = nextPix >> 1;
				ivec2 parentCurrentPix = currentPix >> 1;
				if (parentCurrentPix != parentNextPix){		//if different parents, a parent border is crossed and lods must escalate
					++lod;
					nextPix = parentNextPix; 
					currentPix = parentCurrentPix;
				}
				else{
					break;
				}
			}
			currentPix = nextPix;	
		}
		else if (lod == 0) {
			// hit
			if (rayDepth - texDepth < 0.05){
				FragColor = texelFetch(gColor, currentPix, 0);
			}
			else {
				FragColor = vec4(0.0, 0.0, 1.0, 1.0);		//Blue: Ray occlusion
			}
			return;
		}
		else {
			// refine
			t = t_old;
			--lod;
			currentPix = currentPix << 1;
			bool shiftRight = (ray.start.x + t * ray.dir.x) >= float((currentPix.x + 1) << lod);
			bool shiftUp = (ray.start.y + t * ray.dir.y) >= float((currentPix.y + 1) << lod);
			currentPix += ivec2(shiftRight, shiftUp);
		}
	}
	if (iter == maxIter) {FragColor = vec4(0.0, 1.0, 1.0, 1.0);}  // Cyan: iteration time-out
}
