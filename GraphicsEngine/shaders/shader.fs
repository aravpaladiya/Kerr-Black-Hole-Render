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

struct Light {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform Light light;


void main()
{	
	vec3 ray = light.position-pos;

	vec3 diffuseTex = texture(material.diffuse, texCoords).rgb;

	vec3 normal = normalize(normalsOut);

	vec3 a = diffuseTex * light.ambient;

	vec3 dir = normalize(ray);
	float dScale = max(0.0, dot(normal, dir));
	vec3 d = diffuseTex * dScale * light.diffuse;


	vec3 reflect = -dir + 2 * normal * dot(normal, dir);

	float sScale = pow(max(0.0, dot(normalize(eye-pos), reflect)), material.shininess);
	vec3 s = sScale * light.specular * texture(material.specular, texCoords).rgb;

	FragColor = vec4(a + d + s, 1.0);
	
}

