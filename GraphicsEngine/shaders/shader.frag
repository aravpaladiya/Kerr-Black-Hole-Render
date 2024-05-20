#version 460 core
out vec4 FragColor;
in vec3 pos;
uniform vec3 camPos;
uniform vec3 camRight;
uniform vec3 camUp;
uniform vec3 camDir;

uniform float WIDTH;
uniform float HEIGHT;


uniform sampler2D texture1;

struct Ray {
	vec3 pos;
	vec3 dir;
};

float sqMag(vec3 v) {
	return v.x*v.x + v.y*v.y + v.z*v.z;
}

vec3 acceleration (float h2, vec3 pos) {
	float r5 = pow(sqMag(pos), 2.5);
	return -1.5*h2*pos/r5;
}

vec3 calcColor(Ray ray) {
	float dt = 0.1;
	ray.dir*=dt;
	float h2 = sqMag(cross(ray.pos, ray.dir));
	for (int i = 0; i < 300; i++) {
		ray.dir+=acceleration(h2, ray.pos);
		ray.pos+=ray.dir;
		if (sqMag(ray.pos)<=1) {
			return vec3(0.0);

		}


	}

	ray.dir = normalize(ray.dir);
	ray.dir/=ray.dir.z;
	
	return texture(texture1, vec2(ray.dir.x, ray.dir.y)*0.5+0.5).xyz;
	
} 


void main()
{	
	Ray ray = Ray(camPos, camDir);
	float fov = 90.0;
	float x = pos.x*WIDTH/HEIGHT;
	float y = pos.y;

	ray.dir = normalize(vec3(x, y, 1));
	FragColor = vec4(calcColor(ray), 1.0);
}

