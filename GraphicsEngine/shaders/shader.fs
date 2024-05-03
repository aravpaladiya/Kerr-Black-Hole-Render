#version 450 core
out vec4 FragColor;
in vec3 pos;
in vec3 normalsOut;

uniform vec3 eye;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objColor;

void main()
{	
	vec3 ray = lightPos-pos;


	vec3 normal = normalize(normalsOut);
	float ambStr = 0.1;
	float specStr = 0.5;
	int specScale = 64;
	vec3 a = ambStr * lightColor;
	vec3 dir = normalize(ray);
	float dScale = max(0.0, dot(normal, dir));
	vec3 d = dScale * lightColor;


	vec3 reflect = -dir + 2 * normal * dot(normal, dir);

	float sScale = pow(max(0.0f, dot(normalize(eye-pos), reflect)), specScale);

	vec3 s = sScale * specStr * lightColor;
	FragColor = vec4((a+d+s) * objColor, 1.0);
	
}

