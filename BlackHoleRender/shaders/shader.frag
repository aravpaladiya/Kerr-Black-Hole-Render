#version 460 core
out vec4 FragColor;
in vec3 pos;
uniform vec3 camPos;
uniform vec3 camRight;
uniform vec3 camUp;
uniform vec3 camDir;

uniform float WIDTH;
uniform float HEIGHT;

uniform mat4 lookat;



uniform samplerCube skybox;

struct Ray {
	vec3 pos;
	vec3 dir;
};

float sqMag(vec3 v) {
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

vec3 acceleration (float h2, vec3 pos) {
	//this does not describe the real motion of the photon, but it does give an equivalent trajectory.
	float r5 = pow(sqMag(pos), 2.5);
	return -1.5*h2*pos/r5;
	
}

vec3 acc(float h2, vec3 pos) {
	float r5 = pow(sqMag(pos), 2.5);
	return -1.5*h2*pos/r5;
}

Ray rungekutta(float h2, Ray ray, float h) {
	vec3 k1, k2, k3, k4, l1, l2, l3, l4;

	k1 = ray.dir;
	l1 = acc(h2, ray.pos);

	k2 = ray.dir + h*0.5*l1;
	l2 = acc(h2, h*0.5*k1+ray.pos);

	k3 = ray.dir + h*0.5*l2;
	l3 = acc(h2, h*0.5*k2+ray.pos);

	k4 = ray.dir + h*l3;
	l4 = acc(h2, h*k3+ray.pos);

	return Ray(h/6*(k1+2*k2+2*k3+k4), h/6*(l1+2*l2+2*l3+l4));
	
}



vec3 calcColor(Ray ray) {
	float dt = 0.2;//higher means less accurate, but faster
	float h2 = sqMag(cross(ray.pos, ray.dir));
	//float t = 4*length(ray.pos)*1/dt;
	for (int i = 0; i < round(40/dt); i++) {
		ray+=rungekutta(h2, ray, dt);
		if (sqMag(ray.pos)<=1) {//inside event horizon
			return vec3(0.0, 0.0, 0.0);
		}

	}

//	ray.dir = normalize(ray.dir);
//	ray.dir/=ray.dir.z;
	
	return texture(skybox, ray.dir).rgb;
	
} 


void main()
{	
	//cast ray from camera position
	Ray ray = Ray(camPos, camDir);
	float fov = 90.0;
	float x = pos.x*WIDTH/HEIGHT;
	float y = pos.y;

	ray.dir = normalize(mat3(camRight, camUp, camDir) * vec3(x, y, 1));
	
	FragColor = vec4(calcColor(ray), 1.0);
}

