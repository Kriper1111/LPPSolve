#version 330 core
layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec2 aUV;

out vec2 texCoords;

// FIXME: Make shaders more shared/reusable. Sure, they're small, but still.
uniform mat4 planeTransform = mat4(1.0);
uniform mat4 projection = mat4(1.0);
uniform mat4 transform = mat4(1.0);
uniform mat4 view = mat4(1.0);

void main()
{
    texCoords = vec2(0);
    vec4 transformedPlane = planeTransform * vec4(aPos, 1.0);
    gl_Position = projection * view * transform * transformedPlane;
}