#version 330 core
layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec2 aUV;

out vec2 texCoords;

// FIXME: Boil down planeTransform into one matrix
// FIXME: Translate by normal rather than absolute
uniform mat4 planeTransform = mat4(1.0);
uniform mat4 projection = mat4(1.0);
uniform mat4 view = mat4(1.0);

void main()
{
    texCoords = vec2(0);
    mat4 transform = mat4(1.0);
    vec4 transformedPlane = planeTransform * vec4(aPos, 1.0);
    gl_Position = projection * view * transform * transformedPlane;
}