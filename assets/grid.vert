#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 vertexPosition;
out vec3 worldPosition;
out vec2 texCoords;

uniform vec2 gridOffset[4];
uniform float objectScale = 10.0;
uniform mat4 projection = mat4(1.0);
uniform mat4 view = mat4(1.0);

void main()
{
    vertexPosition = aPos;
    vec3 transformedPosition = ((aPos + vec3(gridOffset[gl_InstanceID], 0.0)) * objectScale) + vec3(0.0, 0.0, -0.005);
    worldPosition = transformedPosition;
    texCoords = vec2(worldPosition);
    gl_Position = projection * view * vec4(transformedPosition, 1.0);
}