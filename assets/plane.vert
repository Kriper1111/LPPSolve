#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

out vec2 texCoords;

uniform mat3 planeTransform = mat3(1.0);
uniform mat4 projection = mat4(1.0);
uniform mat4 view = mat4(1.0);

void main()
{
    texCoords = aUV;
    mat4 transform = mat4(1.0);
    vec3 transformedPlane = planeTransform * aPos;
    gl_Position = projection * view * transform * vec4(transformedPlane, 1.0);
}