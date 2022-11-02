#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 vertexPosition;

uniform mat4 projection = mat4(1.0);
uniform mat4 view = mat4(1.0);

void main()
{
    vertexPosition = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);
}