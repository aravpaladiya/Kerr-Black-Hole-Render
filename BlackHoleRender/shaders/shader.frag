#version 460 core
out vec4 FragColor;
in vec3 pos;
uniform vec3 camPos;
uniform vec3 camRight;
uniform vec3 camUp;
uniform vec3 camDir;

uniform float WIDTH;
uniform float HEIGHT;


float sqDiskIn = 5;
float sqDiskOut = 16;

uniform samplerCube skybox;
#define steps 300
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

vec3 calcColor(vec3 dir) {
	//find quaternion to rotate ray path from ray on equatorial plane
	vec3 a = projectPlane(dir, camDir);
	float costheta = dot(a, camRight)/length(a);
	//rotatea round cam to bh center. rays on plane from that and camdir. in circle.
	float s = sqrt((1-costheta)/2);
;
	float c;

	if (dot(a, camUp)>0) {
		c = sqrt((1+costheta)/2);
	} else {
		c = -sqrt((1+costheta)/2);
	}
	

	vec4 rQ = vec4(s*-camDir, c);

	vec4 rQC = quatConjugate(rQ);
	Ray ray = Ray(camPos, vec3(0));	
	vec3 temp = quatRotate(rQC, dir, rQ);
	float xx = (temp.x * camDir.z - temp.z * camRight.z) / (temp.z * camRight.x - temp.x * camDir.x);

    // Reverse aspect ratio adjustment
    float original_x = xx / (2 * float(WIDTH) / float(HEIGHT));

    // Reverse index calculation
    float x = (original_x + 0.5f) * float(WIDTH);

	//return(vec3(x/WIDTH));

	if (x < 0.0 || x >= WIDTH) {
        return vec3(0.0, 0.0, 1.0); //x outside range
    }
	
	ray.dir = quatRotate(rQ, rayPaths[uint(x)].dir[0], rQC);
	Ray prevRay = ray;
	
	

	
	for (int i = 1; i < 299; i++) {
		prevRay = ray;
		//ray.dir = quatRotate(rQ, rayPaths[uint(x)].dir[i], rQC);
		ray.pos = quatRotate(rQ, rayPaths[uint(x)].pos[i], rQC);
		if (sqMag(ray.pos)<1) {//inside event horizon
			return vec3(1.0, 0.0, 0.0);
		}
		if ((ray.pos.y>0 && ray.pos.y<0) || (prevRay.pos.y<0 && ray.pos.y>0)) {
			float zs = -prevRay.pos.y/(ray.pos.y-prevRay.pos.y);
			float mag = sqMag(prevRay.pos.xz+zs*(ray.pos.xz-prevRay.pos.xz));
			if(sqDiskIn < mag && mag < sqDiskOut) {
				return vec3(0.0, 1.0, 0.0);
			}
		}

	}

	
	return texture(skybox, quatRotate(rQ, rayPaths[uint(x)].dir[299], rQC)).rgb;
	
}


void main()
{	
	//cast ray from camera position
	Ray ray = Ray(camPos, camDir);
	float fov = 90.0;
	float x = pos.x*WIDTH/HEIGHT;
	float y = pos.y;

	ray.dir = normalize(mat3(camRight, camUp, camDir) * vec3(x, y, 1));
	FragColor = vec4(calcColor(ray.dir), 1.0);

}

