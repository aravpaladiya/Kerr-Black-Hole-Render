#version 460 core
out vec4 FragColor;
in vec3 pos;
uniform vec3 camPos;
uniform vec3 camRight;
uniform vec3 camUp;
uniform vec3 camDir;

uniform float WIDTH;
uniform float HEIGHT;
uniform int TOTALCALLS;

uniform float tempconst;
uniform float brightconst;

uniform float time;

float sqDiskIn = 9;
float sqDiskOut = 144;
float diskIn = 3;
float diskOut = 12;
uniform samplerCube skybox;
uniform sampler2D colormap;

#define steps 300
#define PI 3.14159265359
#define angvel 10.0

struct RayPath {
	vec3 pos[steps];
	vec3 dir[steps];
};

layout(std430, binding = 0) buffer RayPathBuffer {
	 RayPath rayPaths[];
};

struct Ray {
	vec3 pos;
	vec3 dir;
};

//disk physics

//blackbody color approximation
//temperature in kelvin to rgb
vec3 blackbodyColor(float temp) {
	vec3 color;

	//red
	if (temp < 6600) {
		color.r = 1.0;
	} else {
		color.r = 1.292936 * pow(temp / 6600.0 - 0.5, -0.1332);
	}

	//green
	if (temp < 6600) {
		color.g = 0.390082 * log(temp / 100.0) - 0.631841;
	} else {
		color.g = 1.129891 * pow(temp / 6600.0 - 0.5, -0.0755);
	}

	//blue
	if (temp < 1900) {
		color.b = 0.0;
	} else if (temp < 6600) {
		color.b = 0.543207 * log(temp / 100.0 - 10.0) - 1.19625;
	} else {
		color.b = 1.0;
	}

	return clamp(color, 0.0, 1.0);
}

//thin disk temperature profile, T ~ r^(-3/4)
float diskTemperature(float r, float rIn) {
	//hotter near center
	return tempconst * pow(rIn / r, 0.75);
}

//hot spots orbiting at their local keplerian velocity, gaussian blobs in (r, theta)

//many small overlapping spots merge into continuous texture
#define NUM_SPOTS 500

float spotHash(float i, float seed) {
	return fract(sin(i * 127.1 + seed * 311.7) * 43758.5453);
}

float diskDensity(float r, float theta, float t) {
	float rNorm = (r - diskIn) / (diskOut - diskIn);

	//base density, smooth falloff at both edges
	float baseDensity = smoothstep(0.0, 0.1, rNorm) * exp(-0.8 * rNorm);

	//spots ordered by radius, spot i at diskIn + (i+0.5)*diskRange/NUM_SPOTS
	//only loop spots within [r-0.5, r+0.5]
	float diskRange = diskOut - diskIn;
	float spotsPerUnit = float(NUM_SPOTS) / diskRange;
	int iMin = max(0, int(floor((r - 0.5 - diskIn) * spotsPerUnit)));
	int iMax = min(NUM_SPOTS, int(ceil((r + 0.5 - diskIn) * spotsPerUnit)));

	float hotspots = 0.0;
	for (int i = iMin; i < iMax; i++) {
		float fi = float(i);

		float spotR = diskIn + (fi + 0.5) * diskRange / float(NUM_SPOTS);

		float dr = r - spotR;

		float spotTheta0 = spotHash(fi, 2.0) * 2.0 * PI;
		float spotRadW = 0.03 + spotHash(fi, 3.0) * 0.07;  //0.03 - 0.10
		float spotAngW = 0.15 + spotHash(fi, 4.0) * 0.25;   //0.15 - 0.40
		float bright = 0.3 + spotHash(fi, 5.0) * 0.7;       //0.3 - 1.0

		//spot orbits at its own keplerian velocity
		float spotOmega = angvel * pow(diskIn / spotR, 1.5);
		float spotTheta = spotTheta0 + spotOmega * (t + 500.0);

		//radial gaussian
		float radial = exp(-dr * dr / (2.0 * spotRadW * spotRadW));

		//angular gaussian, wrap safe via cos
		float dtheta = theta - spotTheta;
		float angular = exp((cos(dtheta) - 1.0) / (spotAngW * spotAngW));

		hotspots += bright * radial * angular;
	}

	return baseDensity * (0.7 + 0.3 * min(hotspots, 1.5));
}


//aces tone mapping, hdr to [0,1) with smooth highlight rolloff
vec3 acesToneMap(vec3 x) {
	float a = 2.51;
	float b = 0.03;
	float c = 2.43;
	float d = 0.59;
	float e = 0.14;
	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

float sqMag(vec3 v) {
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

float sqMag(vec2 v) {
	return v.x*v.x + v.y*v.y;
}

vec3 projectPlane(vec3 v, vec3 planeNormal) {
	return v-planeNormal*dot(v, planeNormal);
}

//quaternions
//real part as 4th element
vec4 quatConjugate(vec4 q) {
    return vec4(-q.x, -q.y, -q.z, q.w);
}

vec4 quatFromAxis(float theta, vec3 v) {
    float s = sin(theta / 2);
    return vec4(s * v.x, s * v.y, s * v.z, cos(theta / 2));
}

vec4 quatMult (vec4 q, vec4 p) {
	return vec4(
		(q.w * p.x) + (q.x * p.w) + (q.y * p.z) - (q.z * p.y),
		(q.w * p.y) - (q.x * p.z) + (q.y * p.w) + (q.z * p.x),
		(q.w * p.z) + (q.x * p.y) - (q.y * p.x) + (q.z * p.w),
		(q.w * p.w) - (q.x * p.x) - (q.y * p.y) - (q.z * p.z)
	);
}

vec3 quatRotate(vec4 q, vec3 p, vec4 qc) {
	return quatMult(quatMult(q, vec4(p, 0)), qc).xyz;
}

vec3 quatRotate(vec4 q, vec3 p) {
	return quatMult(quatMult(q, vec4(p, 0)), quatConjugate(q)).xyz;
}

vec3 minimizeTo1(vec3 v) {
	return vec3(min(1, v.x), min(1, v.y), min(1, v.z));
}


vec3 calcColor(vec3 dir) {
	vec4 texcol = vec4(1, 1, 1, 0);
	vec3 bhDir = -normalize(camPos);

	//find quaternion to rotate ray path from ray on equatorial plane
	vec3 a = projectPlane(dir, bhDir);
	vec3 perpBHDir = normalize(cross(cross(bhDir, camDir), bhDir));
	float costheta = dot(a, perpBHDir)/length(a);

	float s = sqrt((1-costheta)/2);
	vec3 tempCol = vec3(0);
	float c;

	if (dot(a, cross(bhDir, camDir))<0) {
		c = sqrt((1+costheta)/2);
	} else {
		c = -sqrt((1+costheta)/2);
	}

	vec4 rQ = vec4(-s*bhDir, c);
	vec4 rQC = quatConjugate(rQ);

	vec3 b = quatRotate(rQC, dir, rQ);

	Ray ray = Ray(camPos, vec3(0));

	float x = clamp(TOTALCALLS*acos(dot(bhDir, b))/PI, 0.0, float(TOTALCALLS - 1));

	uint xi = uint(x);
	uint xi1 = min(xi + 1u, uint(TOTALCALLS - 1));
	float xfrac = x - float(xi);

	ray.dir = quatRotate(rQ, mix(rayPaths[xi].dir[0], rayPaths[xi1].dir[0], xfrac), rQC);
	Ray prevRay = ray;

	float mag;

	for (int i = 1; i < steps; i++) {
		prevRay = ray;
		vec3 interpPos = mix(rayPaths[xi].pos[i], rayPaths[xi1].pos[i], xfrac);
		ray.pos = quatRotate(rQ, interpPos, rQC);

		if (ray.pos==vec3(0)) {//inside event horizon
			texcol.xyz = vec3(0.0, 0.0, 0.0);
			break;
		}

		//stop early if ray is far past the disk and moving outward
		float rSq = sqMag(ray.pos);
		if (rSq > sqDiskOut * 4.0 && dot(ray.pos, ray.pos - prevRay.pos) > 0.0) {
			break;
		}

		//disk crossing (y sign change)
		if ((prevRay.pos.y > 0 && ray.pos.y < 0) || (prevRay.pos.y < 0 && ray.pos.y > 0)) {
			float zs = -prevRay.pos.y / (ray.pos.y - prevRay.pos.y);
			vec2 diskPos = prevRay.pos.xz + zs * (ray.pos.xz - prevRay.pos.xz);
			mag = sqMag(diskPos);

			if (sqDiskIn < mag && mag < sqDiskOut) {
				float r = sqrt(mag);
				float theta = atan(diskPos.y, diskPos.x);
				if (theta < 0) theta += 2 * PI;

				//smooth edge falloff
				float rNorm = (r - diskIn) / (diskOut - diskIn);
				float edgeFade = smoothstep(0.0, 0.05, rNorm) * smoothstep(1.0, 0.9, rNorm);

				//doppler beaming
				//keplerian orbital velocity v = sqrt(M/(r-r_s)), M=0.5, r_s=1
				float v_orb = sqrt(0.5 / (r - 1.0));

				//tangential velocity, counterclockwise in xz plane
				vec2 radDir = diskPos / r;
				vec2 tanDir = vec2(-radDir.y, radDir.x);
				vec3 v_disk = vec3(tanDir.x, 0.0, tanDir.y) * v_orb;

				//lorentz factor
				float gamma = 1.0 / sqrt(1.0 - v_orb * v_orb);

				//ray direction at the crossing
				vec3 rayDirAtCross = normalize(ray.pos - prevRay.pos);

				//doppler factor g = observed/emitted, g>1 blueshift, g<1 redshift
				//ray is traced outward from camera so material moving with the ray is receding
				float g = 1.0 / (gamma * (1.0 + dot(rayDirAtCross, v_disk)));

				//gravitational redshift
				float camR = length(camPos);
				float g_grav = sqrt((1.0 - 1.0 / r) / (1.0 - 1.0 / camR));
				float gTotal = g * g_grav;

				//temperature and density
				float temperature = diskTemperature(r, diskIn);
				float density = diskDensity(r, theta, time);

				//doppler shift the temperature
				float shiftedTemp = temperature * gTotal;

				//beaming, I_obs = I_emit * g^3
				float beaming = gTotal * gTotal * gTotal;

				//base brightness
				float b = brightconst * (1.0 - 0.7 * rNorm) * edgeFade;

				//colormap lookup with the shifted temperature
				float tempNorm = clamp((shiftedTemp - 1000.0) / 28800.0, 0.02, 0.98);
				vec3 diskColor = texture(colormap, vec2(1.0 - tempNorm, 0.5)).xyz;

				tempCol += diskColor * b * density * beaming;
			}
		}
	}

	//skybox for rays that escape
	texcol.xyz = (texcol.xyz == vec3(0, 0, 0)) ? vec3(0, 0, 0) : texture(skybox, quatRotate(rQ, mix(rayPaths[xi].dir[steps-1], rayPaths[xi1].dir[steps-1], xfrac), rQC)).rgb;

	//tone map the disk emission then add to skybox
	vec3 mappedDisk = acesToneMap(tempCol);
	return texcol.xyz + mappedDisk;
}

void main()
{
	Ray ray = Ray(camPos, camDir);
	float x = pos.x * WIDTH / HEIGHT;
	float y = pos.y;

	ray.dir = normalize(mat3(camRight, camUp, camDir) * vec3(x, y, 1));
	FragColor = vec4(calcColor(ray.dir), 1.0);
}
