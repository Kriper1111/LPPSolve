#version 330 core
out vec4 FragColor;

in vec2 texCoords;

uniform float stripeScale = 10.0;
uniform float stripeWidth = 0.5;
uniform vec3 positiveColor = vec3(0.0, 0.0, 1.0);
uniform vec3 negativeColor = vec3(1.0, 0.0, 0.0);

void main()
{
    float gradient = ((texCoords.x + texCoords.y) / 2.0) * stripeScale;
    gradient = floor(fract(gradient) + stripeWidth);
    if (gradient == 0.0) discard;
    vec3 fragColorNoAlpha = mix(positiveColor, negativeColor, float(gl_FrontFacing));
    FragColor = vec4(fragColorNoAlpha, 1.0);
}