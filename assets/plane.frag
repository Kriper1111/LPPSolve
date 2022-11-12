#version 330 core
out vec4 FragColor;

in vec2 texCoords;

uniform float stripeScale = 10.0;
uniform float stripeWidth = 0.5;
uniform sampler2D imageTexture;
uniform vec3 vertexColor = vec3(1.0, 1.0, 1.0);

void main()
{
    float gradient = ((texCoords.x + texCoords.y) / 2.0) * stripeScale;
    gradient = floor(fract(gradient) + stripeWidth);
    if (gradient == 0.0) discard;
    FragColor = vec4(1.0 - float(gl_FrontFacing), 0.0, gl_FrontFacing, 1.0);
}