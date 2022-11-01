#version 330 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D imageTexture;
uniform vec3 vertexColor = vec3(1.0, 1.0, 1.0);

void main()
{
    // vec2 centerPoint = vec2(1.0, 1.0);
    // vec2 uv = texCoords - 0.5;
    // float dist = 0.25 - dot(uv, uv);
    // FragColor = vec4(dist * 1.0, dist * 1.0, dist * 1.0, 1.0);
    FragColor = vec4(texCoords, gl_FrontFacing, 1.0);
}