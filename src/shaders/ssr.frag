#version 330 core
// TODO: 
//			1/4, 1/16, 1/64 resolution, neighbour filtering, temporal smoothing
//			multiple rays per pixel with sleight offset, mostly coherent, so progress together
//			downsample reflection buffer and use for rough surfaces
//			set min forward angle (now perpendicular rays are permitted)
//			better hit rejection heuristics
//			methods to mitigate thin object and foreground artefacts (neighbour filter, forward sample)
//			fallback marker (0.0 in reflection buffer?)
//			get safe cell by remembering t1 or t2 to know if on a horo or vert edge and nudge accordingly

out vec4 FragColor;

in vec2 TexCoords;
in vec3 ViewDir;

uniform sampler2D gDepth;
uniform sampler2D gNormal;
uniform sampler2D gColor;		//use renderbuffer from last frame as a proxy for the pixel colour

uniform int scr_height;
uniform float aspect_ratio;
uniform float tanHalfFov;

uniform float near;
uniform float far;

uniform mat4 projection;
uniform mat4 inverseProjection;
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

ScreenRay setupRay(vec3 viewPos, vec3 R, vec3 N){
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

float textureDepth(vec2 tc, int lod)
{
	float depth = textureLod(gDepth, tc, float(lod)).r;
    float ndc = 2.0 * depth - 1.0;
    float linDepth = abs((2.0 * near * far)/(far + near - ndc * (far - near)));
	return linDepth;
}

vec3 getViewPosOld()
{
	vec4 ndc = vec4((2.0 * TexCoords - 1.0), 0.0, 1.0);

	float xOffset = ndc.x * aspect_ratio * tanHalfFov;
    float yOffset = ndc.y * tanHalfFov;

	float linDepth = textureDepth(TexCoords, 0);

	vec3 viewDir = vec3(xOffset, yOffset, -1.0);

	return viewDir * linDepth;
}

vec3 getViewPos(vec2 uv)
{
    // 1. Sample the raw, non-linear depth from the depth texture
    float rawDepth = texture(gDepth, uv).r;
	
    // 2. Convert UV and raw depth into Normalized Device Coordinates (NDC)
    // This maps X, Y, and Z perfectly into the [-1.0, 1.0] range (OpenGL Standard)
    vec4 ndc = vec4(
        uv.x * 2.0 - 1.0,
        uv.y * 2.0 - 1.0,
        rawDepth * 2.0 - 1.0,
        1.0
    );

    // 3. Transform the NDC point back into Homogeneous View Space
    vec4 viewPosH = inverseProjection * ndc;

    // 4. Perform the perspective divide to resolve the true 3D View Space position
    return viewPosH.xyz / viewPosH.w;
}

vec3 getViewPos()
{
	float linDepht = textureDepth(TexCoords, 0);
	return ViewDir * (linDepht / ViewDir.z);
}

vec2 nudge(vec2 position, ScreenRay ray){
	vec2 nudgePos = position + sign(ray.dir) * 1e-6;
	return nudgePos;
}

vec2 boundaryIntersect(int lod, ScreenRay ray, vec2 currentPos)
{
	ivec2 texRes = textureSize(gDepth, lod);
	//float vertRes = float(scr_height) / pow(2.0, float(lod));						// this can be precomputed in an array at start
	//float horiRes = (float(scr_height) * aspect_ratio) / pow(2.0, float(lod));		// this can be precomputed in an array at start
	//float dx = 1.0 / horiRes;														// this can be precomputed in an array at start
	//float dy = 1.0 / vertRes;														// this can be precomputed in an array at start
	float dx = 1.0 / texRes.x;													// this can be precomputed in an array at start
	float dy = 1.0 / texRes.y;
	vec2 nudgePos = currentPos + sign(ray.dir) * 1e-7;
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


// Idea: trace at 1/16 or 1/64 res, check if hitpos are coherent (normal, depth), 
// if so interpolate, if not, retrace. This could even igonre thin objects from blocking rays
void main()
{
	vec3 test = vec3(0.0);
	vec3 viewPos = getViewPos(TexCoords); 

	vec3 V = normalize(viewPos);
	//vec3 V = normalize(ViewDir);												// viewspace
	vec3 N = normalize(normalView * texture(gNormal, TexCoords).rgb);			// viewspace
	vec3 R = normalize(reflect(V, N));											// viewspace
	
	//vec3 viewPos = getViewPos(); 
	ScreenRay ray = setupRay(viewPos, R, N);
	
	int lod = maxLod;
	vec2 lodExit[16];
	int iter = 0;
	bool hit = false;
	bool offscreen = false;

	vec2 currentPos = ray.start;
	float depth = rayDepth(ray, currentPos);
	currentPos = boundaryIntersect(0, ray, currentPos);
	depth = rayDepth(ray, currentPos);

	if (R.z >= 0){
		//FragColor = vec4(0.6, 0.0, 0.02, 1.0);
		offscreen = true;
	}
	// Ray life
	while (!hit && !offscreen && (iter < maxIter))
	{
		// begin test
		ivec2 texRes = textureSize(gDepth, lod);
		//float vertRes = float(scr_height) / pow(2.0, float(lod));						// this can be precomputed in an array at start
		//float horiRes = (float(scr_height) * aspect_ratio) / pow(2.0, float(lod));		// this can be precomputed in an array at start
		//float dx = 1.0 / horiRes;														// this can be precomputed in an array at start
		//float dy = 1.0 / vertRes;														// this can be precomputed in an array at start
		float dx = 1.0 / texRes.x;													// this can be precomputed in an array at start
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
		//if (t1 < 0.0) { t1 = 1e30; }
		//if (t2 < 0.0) { t2 = 1e30; }
		float t = min(t1, t2);
		// end test


		vec2 exitPos = boundaryIntersect(lod, ray, currentPos);
		
		vec2 exitDist = (exitPos - currentPos);
		vec2 nudgeDist = (nudgePos - currentPos);
		//test = vec3(100.0 * exitDist, 0.0);
		//test = vec3(100.0 * nudgeDist, 0.0);
		//test = vec3(ray.dir, 0.0);
		//test = vec3(exitPos.y, currentPos.y, 0.0);
		//test = vec3(10000 * (exitPos.y - currentPos.y), 10000 * (nudgePos.y - currentPos.y), 10000 * dy);
		float maxRayDepth = rayDepth(ray, exitPos);
		lodExit[lod] = exitPos;
		vec2 samplePos = (currentPos + exitPos) * 0.5;
		vec2 sampleDist = (samplePos - currentPos);
		//test = vec3(10000.0 * sampleDist, 0.0);
		//test = 1000.0 * vec3(length(sampleDist), length(exitDist), length(nudgeDist));
		float texDepth = textureDepth(samplePos, lod);
		//test = vec3(depth, depth - maxRayDepth, depth - texDepth);
		//test = vec3(depth, maxRayDepth, texDepth);
		if ((maxRayDepth > texDepth) && (lod != 0))
		{
			// Refine search
			lod -= 1;
		}
		else if ((maxRayDepth > texDepth) && (lod == 0))
		{
			// Ray hit found
			hit = true;
			float minRayDepth = rayDepth(ray, currentPos);
			float interp = clamp((texDepth - minRayDepth)/max(maxRayDepth - minRayDepth, 1e-10), 0.0, 1.0);
			vec2 hitPos = mix(currentPos, exitPos, interp);
			hitPos = currentPos;
			//test = vec3(iter/1000.0);
			//FragColor = vec4(0.0, 0.0, 1.0, 1.0);			//blue
			// do some checks before assigning color; depth delta too big, 
			if (minRayDepth < texDepth * 1.005){
				FragColor = vec4(texture(gColor, hitPos).rgb, 1.0);
				//FragColor = vec4(0.0, 0.0, 1.0, 1.0);			//blue
			}
			else{
				FragColor = vec4(test, 1.0);
				//FragColor = vec4(0.0, 0.0, 1.0, 1.0);		//blue
			}			
			//FragColor = vec4(texture(gColor, hitPos).rgb, 1.0);
			// consider resuming search if depth hit is thin, (this allows to see behind thin opjects, 
			// but will break when the thin object is in fact solid in a large z direciton e.g. a thin z alligned wall)
			break;
		}
		else
		{
			// No hit on current lod
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
			currentPos = exitPos;
			depth = maxRayDepth;
			if ((currentPos.x > 0.999999) || (currentPos.y > 0.999999) 
			 || (currentPos.x < 0.000001) || (currentPos.y < 0.000001)){
				offscreen = true;   // screen boundary check
				//FragColor = vec4(1.0, 0.0, 1.0, 1.0);
				break;
			}
			//for (int i=0; i<maxLod+1; ++i){
			//	if (length(currentPos - lodExit[i]) < 1e-6){//pow(1.0, -(maxLod + 2))){		//use pixel index instead of lod to guarantee equality
			//		lod = i;		// shift up in lod level
			//	}
			//}
		}
		iter++;
	}
	if (!hit && !offscreen) {
		//FragColor = vec4(0.0, 0.8, 0.0, 1.0);		// green
		//FragColor = vec4(test, 1.0);
	}
	//if (offscreen) {
	//	FragColor = vec4(1.0, 0.0, 0.0, 1.0);		// red
	//}
	test = vec3(FragColor);
	//test = vec3(ray.dir, 0.0);
	vec3 oldtest = test;
	if ((abs(TexCoords.x - 1.0/2.0) < 0.01) && (abs(TexCoords.y - 1.0/2.0) < 0.01)){
		test = vec3(0.0, 1.0, 0.0);
	}
	if ((abs(TexCoords.x - 1.0/2.0) < 0.005) && (abs(TexCoords.y - 1.0/2.0) < 0.005)){
		test = oldtest;
		
	}
	FragColor = vec4(test, 1.0);
	//FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
