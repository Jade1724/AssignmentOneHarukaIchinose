#version 330

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
uniform mat4 mvMatrix;
uniform mat4 norMatrix;
uniform vec4 lightPos;

out vec4 lighting;

void main()
{
   gl_Position = position;
}