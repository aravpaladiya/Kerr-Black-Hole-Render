#version 460 core
out vec4 FragColor;
in vec3 pos;

uniform int WIDTH, HEIGHT;
uniform float time;
uniform float spin_a;
uniform float tempconst;
uniform float brightconst;
uniform float diskIn;   //isco radius
uniform float diskOut;

uniform samplerCube skybox;
uniform sampler2D colormap;

#define PI 3.14159265359
#define MAX_CROSSINGS 3
#define NUM_SPOTS 500
#define angvel 10.0

struct PixelResult {
	vec4 skyDir_hitType;
	vec4 crossing0;
	vec4 crossing1;
	vec4 crossing2;
};

layout(std430, binding = 0) buffer PixelBuffer {
	PixelResult pixels[];
};

//accretion disk

//aces tone mapping
vec3 acesToneMap(vec3 x) {
	float a_ = 2.51;
	float b_ = 0.03;
	float c_ = 2.43;
	float d_ = 0.59;
	float e_ = 0.14;
	return clamp((x * (a_ * x + b_)) / (x * (c_ * x + d_) + e_), 0.0, 1.0);
}

//thin disk temperature profile, T ~ r^(-3/4)
float diskTemperature(float r) {
	return tempconst * pow(diskIn / r, 0.75);
}

//hash for procedural spots
float spotHash(float i, float seed) {
	return fract(sin(i * 127.1 + seed * 311.7) * 43758.5453);
}

//density with orbiting hot spots
float diskDensity(float r, float theta, float t) {
	float diskRange = diskOut - diskIn;
	float rNorm = (r - diskIn) / diskRange;
	float baseDensity = smoothstep(0.0, 0.1, rNorm) * exp(-0.8 * rNorm);

	//only loop nearby spots
	float spotsPerUnit = float(NUM_SPOTS) / diskRange;
	int iMin = max(0, int(floor((r - 0.5 - diskIn) * spotsPerUnit)));
	int iMax = min(NUM_SPOTS, int(ceil((r + 0.5 - diskIn) * spotsPerUnit)));

	float hotspots = 0.0;
	for (int i = iMin; i < iMax; i++) {
		float fi = float(i);
		float spotR = diskIn + (fi + 0.5) * diskRange / float(NUM_SPOTS);
		float dr = r - spotR;

		float spotTheta0 = spotHash(fi, 2.0) * 2.0 * PI;
		float spotRadW = 0.03 + spotHash(fi, 3.0) * 0.07;
		float spotAngW = 0.15 + spotHash(fi, 4.0) * 0.25;
		float bright = 0.3 + spotHash(fi, 5.0) * 0.7;

		//kerr keplerian angular velocity, matching the schwarzschild convention
		//schwarzschild angvel*(diskIn/spotR)^1.5, kerr angvel*diskIn^1.5/(spotR^1.5 + a)
		float spotOmega = angvel * pow(diskIn, 1.5) / (pow(spotR, 1.5) + spin_a);
		float spotTheta = spotTheta0 + spotOmega * (t + 500.0);

		float radial = exp(-dr * dr / (2.0 * spotRadW * spotRadW));
		float dtheta = theta - spotTheta;
		float angular = exp((cos(dtheta) - 1.0) / (spotAngW * spotAngW));

		hotspots += bright * radial * angular;
	}

	return baseDensity * (0.7 + 0.3 * min(hotspots, 1.5));
}

void main() {
	uint ix = uint(gl_FragCoord.x);
	uint iy = uint(gl_FragCoord.y);
	if (ix >= WIDTH || iy >= HEIGHT) {
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}
	uint idx = iy * uint(WIDTH) + ix;

	PixelResult px = pixels[idx];

	//debug tile visualization
	float hitW = px.skyDir_hitType.w;
	if (hitW >= 10.0) {
		int level = int(hitW) - 10;
		vec3 colors[4] = vec3[4](
			vec3(0.0, 0.8, 0.0),   //level 0 green (8x8 tile)
			vec3(0.8, 0.8, 0.0),   //level 1 yellow (4x4)
			vec3(0.8, 0.4, 0.0),   //level 2 orange (2x2)
			vec3(0.8, 0.0, 0.0)    //level 3 red (1x1 per pixel)
		);
		FragColor = vec4(colors[clamp(level, 0, 3)], 1.0);
		return;
	}

	bool isHorizon = hitW > 1.0;

	//skybox only for escaped rays, black for horizon
	vec3 skyCol = isHorizon ? vec3(0.0) : texture(skybox, px.skyDir_hitType.xyz).rgb;


	//accumulate disk emission from all crossings, even if ray later hit horizon
	vec3 diskEmission = vec3(0.0);
	vec4 crossings[3] = vec4[3](px.crossing0, px.crossing1, px.crossing2);
	int numCrossings = int(px.crossing0.w);

	for (int c = 0; c < MAX_CROSSINGS; c++) {
		if (c >= numCrossings) break;

		float r_cross   = crossings[c].x;
		float phi_cross = crossings[c].y;
		float gTotal    = crossings[c].z;

		//reject invalid crossings from interpolation artifacts
		if (r_cross < diskIn - 1.0 || r_cross > diskOut + 1.0) continue;
		if (gTotal <= 0.0 || gTotal > 10.0) continue; //clamp extreme g factors

		float rNorm = (r_cross - diskIn) / (diskOut - diskIn);
		//wider inner fade to smooth interpolation boundaries
		float edgeFade = smoothstep(-0.03, 0.08, rNorm) * smoothstep(1.03, 0.9, rNorm);

		float temperature = diskTemperature(r_cross);
		float density = diskDensity(r_cross, phi_cross, time);

		float shiftedTemp = temperature * gTotal;
		float beaming = gTotal * gTotal * gTotal;

		float b = brightconst * (1.0 - 0.7 * rNorm) * edgeFade;

		float tempNorm = clamp((shiftedTemp - 1000.0) / 28800.0, 0.02, 0.98);
		vec3 diskColor = texture(colormap, vec2(1.0 - tempNorm, 0.5)).xyz;

		diskEmission += diskColor * b * density * beaming;
	}

	vec3 mappedDisk = acesToneMap(diskEmission);
	FragColor = vec4(skyCol + mappedDisk, 1.0);
}
