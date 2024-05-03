#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normals;

out vec3 normalsOut;
out vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{   
    vec3 fragPos = vec3(model * vec4(aPos, 1.0));
    normalsOut = mat3(transpose(inverse(model)))*normals;
    pos = fragPos;
    gl_Position = projection * view * vec4(fragPos, 1.0);
}