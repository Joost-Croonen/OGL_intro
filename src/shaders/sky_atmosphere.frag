#version 460 core
#define PI 3.14159265358979323846
out vec4 FragColor;

in vec3 ViewDir;

uniform vec3 sunDir;
uniform sampler2D atmosphereLUT;
uniform vec3 camPos;
uniform vec3 planetPos;

const float factorR = 1.0;
const float factorM = 1.0;
const float factorO = 1.0;

const int NUMSTEPS = 128;

const vec3 rayleighScatter = factorR * vec3(5.8e-3, 13.5e-3, 33.1e-3);
const vec3 mieScatter = factorM * vec3(4.0e-3);
const vec3 mieExtinct = mieScatter * 1.11;
const vec3 ozoneExtinct = factorO * vec3(0.000650, 0.001881, 0.000085);
const vec3 sunColor = 10.0 * vec3(1.0, 1.0, 1.0);	
const float R = 6371.0;
const float hmax = 80.0;
const float HR = 8.0;
const float HM = 1.2;
const float HOB = 15.0;
const float HOT = 35.0;
const float HOM = (HOB + HOT) * 0.5;

// Rayleigh Phase Function
float phaseRayleigh(float cosTheta) {
    return (3.0 / (16.0 * PI)) * (1.0 + cosTheta * cosTheta);
}

// Mie Phase Function (Henyey-Greenstein, g ~ 0.76 to 0.85)
float phaseMie(float cosTheta, float g) {
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * PI * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}

vec3 sphereUp(vec3 c, float r, vec3 d, vec3 o)
{
    vec3 oc = c - o;
    if (length(oc) < r)
        return normalize(o - c);
    float t = max(dot(oc, d), 0.0);
    vec3 x = t * d + o;
    float h = length(x - c);
    if (h > r){
        return normalize(x - c);
    }
    else {
        float s = sqrt(r*r - h*h);
        vec3 p = (t - s) * d + o;
        return normalize(p - c);
    }
}

float sphereHeight(vec3 c, float r, vec3 d, vec3 o)
{
	vec3 oc = c - o;
    if (length(oc) < r)
        return length(o - c);
    float t = max(dot(oc, d), 0.0);
    vec3 x = t * d + o;
    float h = length(x - c);
    if (h > r){
        return length(x - c);
    }
    else {
        float s = sqrt(r*r - h*h);
        vec3 p = (t - s) * d + o;
        return length(p - c);
    }
}

vec3 sphereEntry(vec3 c, float r, vec3 d, vec3 o)
{
	vec3 oc = c - o;
    if (length(oc) < r)
        return o;
    float t = max(dot(oc, d), 0.0);
    vec3 x = t * d + o;
    float h = length(x - c);
    if (h > r){
        return vec3(0.0);
    }
    else {
        float s = sqrt(r*r - h*h);
        vec3 p = (t - s) * d + o;
        return p;
    }
}

vec3 sphereExit2(vec3 c, float ra, float rp, vec3 d, vec3 o)
{
	vec3 oc = c - o;
    float t = dot(oc, d);
    vec3 x = t * d + o;
    float h = length(x - c);
    float sa = sqrt(ra*ra - h*h);
    float ta = (t + sa);
    float sp = sqrt(rp*rp - h*h);
    float tp = (t - sp);
	if (tp<0.0){
		vec3 p = ta * d + o;
		return p;
	}
	else {
		vec3 p = min(ta, tp) * d + o;
		return p;
	}
}


vec3 sphereExit(vec3 c, float ra, float rp, vec3 d, vec3 o)
{
	vec3 oc = c - o;
    float t = dot(oc, d);
    vec3 x = t * d + o;
    float h = length(x - c);
    
	//if (length(oc) < ra){	// in atmosphere
	//	float test = 0.0;
	//}
	//else{					// out atmosphere
		if (h > rp){
			float s = sqrt(ra*ra - h*h);
			vec3 p = (t + s) * d + o;
			return p;
		}
		else if (length(oc) < ra){
			
		}
		else {
			vec3 p = sphereEntry(c, rp, d, o);
			return p;
		}
	//}
}

float[NUMSTEPS] distributeSteps(vec3 c, float ra, float rp, vec3 d, vec3 o, float h0)
{
	const int numsteps = NUMSTEPS;
	float[numsteps] distribtion;
	
	if (h0<0.0)
		return distribtion;

	vec3 p1 = sphereEntry(c, ra, d, o);
	vec3 p2 = sphereExit2(c, ra, rp, d, o);

	float h1 = length(p1);
	float h2 = length(p2);

	float t1 = dot(p1 - o, d);
	float t2 = dot(p2 - o, d);
	
	float tl = t2 - t1;

	float to = t1;

	for (int i = 0; i<numsteps; i++){
		float t = t1 + tl/numsteps * i;
		vec3 p = d*t + o;
		float sgn = -sign(dot(p - c, d));
		float normLen = (length(p - c) - rp)  / (ra - rp);
		float s = sgn * (1.0 - sqrt(normLen)) * tl / numsteps; 
		float tn = max(t*s, to);
		distribtion[i] = max(tn - to, 0.1);
		to = tn;
		float a = sqrt(normLen);
		distribtion[i] = a * tl - to;
	}


	return distribtion;
}

void main()
{

	vec3 CamPos = camPos - planetPos;
	vec3 PlanetPos = vec3(0.0);
	const vec3 V = normalize(ViewDir);
	const vec3 L = normalize(-sunDir);
	const vec3 U = sphereUp(PlanetPos, R + hmax, V, CamPos);

	// sun disk
	const float LdotV = max(dot(L, V), 0.001);
	//float sunStrenght = pow(LdotV, 64.0) / 15.0;
	//sunStrenght = 1000*exp(-(LdotV-1)*(LdotV-1)/0.00000001);

	float sunDisk = smoothstep(0.9997, 0.9999, LdotV);
	float sunGlow = 0.0 * pow(LdotV, 128.0) / 20.0;

	vec3 color = (sunDisk + sunGlow) * sunColor;

	// sun scattering
	const float sinAlpha = dot(V, U);//dot(V, U);//V.y;
	float h0 = sphereHeight(PlanetPos, R + hmax, V, CamPos) - R;
	vec2 lutUV = vec2(sinAlpha * 0.5 + 0.5, clamp(h0, 0, hmax)/hmax);
	vec3 density = texture(atmosphereLUT, lutUV).rgb;
	vec3 opticalDepth = rayleighScatter * density.x + mieExtinct * density.y + ozoneExtinct * density.z;
	vec3 transmittance =  exp(-opticalDepth);
	if (h0>81.0)
		transmittance = vec3(1.0);
	color *= transmittance;

	// sky inscatter
	const int numSteps = NUMSTEPS;
	const float stepPower = 1.0;//1.124;//1.06;
	//const vec3 Pstart = vec3(0.0, R, 0.0);

	float pathlenght = hmax;
	if (h0>0.0){
		vec3 p1 = sphereEntry(planetPos, R + hmax, V, camPos);
		vec3 p2 = sphereExit2(planetPos, R + hmax, R, V, camPos);
		pathlenght = length(p1 - p2);
	}

	//float stepSizes[numSteps] = distributeSteps(PlanetPos, R+hmax, R, V, CamPos, h0);

	const float cosTheta = dot(V, L);
	const float phaseR = phaseRayleigh(cosTheta);
	const float phaseM = phaseMie(cosTheta, 0.8);
	const float sinAlphaSun = dot(L, U);//L.y;
	const float maxLength = 2 * sqrt(20*R*HR + 100*HR*HR);
	float t = 0.0;
	float dt = 0.1;
	float viewDensityR = 0.0;
	float viewDensityM = 0.0;
	float viewDensityO = 0.0;
	vec3 transmittanceView = vec3(0.0);
	vec3 skyAccumulated = vec3(0.0);
	for (int i = 0; i<numSteps; i++){
		t += dt;
		float ht = sqrt(t*t + R*R + 2.0*R*h0 + h0*h0 + 2*t*(R+h0)*sinAlpha) - R;
		if (ht < 0.0) break;
		if (ht >= 80.0) continue;
		//vec3 P = Pstart + V*t;
		//float ht = length(P) - R;
		vec2 lutUVsun = vec2(sinAlphaSun * 0.5 + 0.5, ht/hmax);
		vec3 sunDensity = texture(atmosphereLUT, lutUVsun).rgb;
		vec3 sunOpticalDepth = rayleighScatter * sunDensity.x + mieExtinct * sunDensity.y + ozoneExtinct * sunDensity.z;
		vec3 transmittanceSun =  exp(-sunOpticalDepth);

		float densityR = exp(-ht/HR);
		float densityM = exp(-ht/HM);
		float densityO = smoothstep(HOB, HOM, ht) * (1.0 - smoothstep(HOM, HOT, ht));
		viewDensityR += densityR * dt;
		viewDensityM += densityM * dt;
		viewDensityO += densityO * dt;
		vec3 viewOpticalDepth = rayleighScatter * viewDensityR + mieExtinct * viewDensityM + ozoneExtinct * viewDensityO;
		transmittanceView = exp(-viewOpticalDepth);

		vec3 scattering = (rayleighScatter * phaseR * densityR + mieScatter * phaseM * densityM) * sunColor;

		skyAccumulated += transmittanceSun * transmittanceView * scattering * dt;

		dt *= stepPower;
		dt = 2000.0/numSteps * (1.1 - abs(sinAlphaSun)) * exp(ht/hmax);
		dt = pathlenght/numSteps;
		//dt = stepSizes[i];
	}
	color += skyAccumulated;
	//color += underSurfae * vec3(0.1, 0.2, 0.1) * max(dot(U, L), 0.0);

	//color = color / (color + vec3(1.0));		// tone mapping
	//color = pow(color, vec3(1.0/2.2));			// gamma correction
	float scalarTransmittance = (transmittanceView.r + transmittanceView.g + transmittanceView.b) / 3.0;
	FragColor = vec4(color, scalarTransmittance);
}