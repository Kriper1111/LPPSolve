#version 330 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D imageTexture;
uniform vec3 vertexColor = vec3(1.0, 1.0, 1.0);

void main()
{
    FragColor = vec4(gl_FrontFacing, 0.0, 1.0 - float(gl_FrontFacing), 1.0);
}