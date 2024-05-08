#version 450 core
out vec4 FragColor;
in vec3 pos;
in vec3 normalsOut;

uniform vec3 eye;
uniform vec3 objColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform bool flashlight;

in vec2 texCoords;


struct Spotlight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	float cutoffIn;
	float cutoffOut;
	vec3 direction;

};

struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};






uniform PointLight pointLight;

uniform Spotlight spotlight;

vec3 calcSpotlight (Spotlight light, vec3 normal, vec3 pos, vec3 viewDir) {
	vec3 direc = normalize(light.direction);

	float cosAngle = dot(direc, normalize(pos-light.position));

	float cutoffScale = clamp((cosAngle-light.cutoffOut)/(light.cutoffIn-light.cutoffOut), 0, 1);

	vec3 ray = light.position-pos;

	float dist = length(ray);

	vec3 diffuseTex = texture(texture_diffuse1, texCoords).rgb;

	vec3 a = diffuseTex * light.ambient;

	vec3 dir = normalize(ray);
	float dScale = max(0.0, dot(normal, dir));
	vec3 d = diffuseTex * dScale * light.diffuse;

	float attenuation = 1/(light.constant + light.linear * dist + light.quadratic * dist * dist);

	vec3 reflect = -dir + 2 * normal * dot(normal, dir);

	float sScale = pow(max(0.0, dot(normalize(-viewDir), reflect)), 32.0);
	vec3 s = sScale * light.specular * texture(texture_specular1, texCoords).rgb;

	a*=cutoffScale;
	d*=cutoffScale;
	s*=cutoffScale;
	vec3 result = attenuation * (a + d + s);

	return result;
}


vec3 calcPointLight (PointLight light, vec3 normal, vec3 pos, vec3 viewDir) {
	vec3 ray = light.position-pos;

	float dist = length(ray);

	vec3 diffuseTex = texture(texture_diffuse1, texCoords).rgb;

	vec3 a = diffuseTex * light.ambient;

	vec3 dir = normalize(ray);
	float dScale = max(0.0, dot(normal, dir));
	vec3 d = diffuseTex * dScale * light.diffuse;

	float attenuation = 1/(light.constant + light.linear * dist + light.quadratic * dist * dist);

	vec3 reflect = -dir + 2 * normal * dot(normal, dir);

	float sScale = pow(max(0.0, dot(normalize(-viewDir), reflect)), 32.0);
	vec3 s = sScale * light.specular * texture(texture_specular1, texCoords).rgb;

	vec3 result = attenuation * (a + d + s);

	return result;
}


void main()
{	
	vec3 n = normalize(normalsOut);
    
	vec3 viewDir = pos-eye;

	
	vec3 r = calcPointLight(pointLight, n, pos, viewDir);
	if (flashlight) {
		r+= calcSpotlight(spotlight, n, pos, viewDir);
	}
	
	FragColor = vec4(r, 1.0);

	
}

