#version 450 core
out vec4 FragColor;
in vec3 pos;
in vec3 normalsOut;

uniform vec3 eye;
uniform vec3 objColor;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

in vec2 texCoords;

struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

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

struct DirLight {
	vec3 direction;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform DirLight dirLight;
uniform Spotlight spotlight;


uniform PointLight pointLights[4];

vec3 calcPointLight (PointLight light, vec3 normal, vec3 pos, vec3 viewDir) {
	vec3 ray = light.position-pos;

	float dist = length(ray);

	vec3 diffuseTex = texture(material.diffuse, texCoords).rgb;

	vec3 a = diffuseTex * light.ambient;

	vec3 dir = normalize(ray);
	float dScale = max(0.0, dot(normal, dir));
	vec3 d = diffuseTex * dScale * light.diffuse;

	float attenuation = 1/(light.constant + light.linear * dist + light.quadratic * dist * dist);

	vec3 reflect = -dir + 2 * normal * dot(normal, dir);

	float sScale = pow(max(0.0, dot(normalize(-viewDir), reflect)), material.shininess);
	vec3 s = sScale * light.specular * texture(material.specular, texCoords).rgb;

	vec3 result = attenuation * (a + d + s);

	return result;
}

vec3 calcDirLight (DirLight light, vec3 normal, vec3 viewDir) {
	vec3 direc = normalize(light.direction);

	vec3 diffuseTex = texture(material.diffuse, texCoords).rgb;

	vec3 a = diffuseTex * light.ambient;

	float dScale = max(0.0, dot(normal, -direc));
	vec3 d = diffuseTex * dScale * light.diffuse;

	vec3 reflect = direc - 2 * normal * dot(normal, direc);
	float sScale = pow(max(0.0, dot(normalize(-viewDir), reflect)), material.shininess);
	vec3 s = sScale * light.specular * texture(material.specular, texCoords).rgb;
	vec3 result = a + d + s;
	return result;
}

vec3 calcSpotlight (Spotlight light, vec3 normal, vec3 pos, vec3 viewDir) {
	vec3 direc = normalize(light.direction);

	float cosAngle = dot(direc, normalize(pos-light.position));

	float cutoffScale = clamp((cosAngle-light.cutoffOut)/(light.cutoffIn-light.cutoffOut), 0, 1);

	vec3 ray = light.position-pos;

	float dist = length(ray);

	vec3 diffuseTex = texture(material.diffuse, texCoords).rgb;

	vec3 a = diffuseTex * light.ambient;

	vec3 dir = normalize(ray);
	float dScale = max(0.0, dot(normal, dir));
	vec3 d = diffuseTex * dScale * light.diffuse;

	float attenuation = 1/(light.constant + light.linear * dist + light.quadratic * dist * dist);

	vec3 reflect = -dir + 2 * normal * dot(normal, dir);

	float sScale = pow(max(0.0, dot(normalize(-viewDir), reflect)), material.shininess);
	vec3 s = sScale * light.specular * texture(material.specular, texCoords).rgb;

	a*=cutoffScale;
	d*=cutoffScale;
	s*=cutoffScale;
	vec3 result = attenuation * (a + d + s);

	return result;
}

void main()
{	
	vec3 n = normalize(normalsOut);
    
	vec3 viewDir = pos-eye;

	vec3 dirResult = calcDirLight(dirLight, n, viewDir);
	vec3 spotResult = calcSpotlight(spotlight, n, pos, viewDir);
	
	vec3 r = spotResult + dirResult;
	for (int i = 0; i < 4; i++)
		r += calcPointLight(pointLights[i], n, pos, viewDir);
	
	FragColor = vec4(r, 1.0);

	
}

