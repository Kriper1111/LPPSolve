#version 330 core
out vec4 FragColor;

in vec2 texCoords;

uniform vec3 vertexColor = vec3(1.0, 1.0, 1.0);
uniform float gridScale = 1.0;
uniform float strokeWidth = 0.1;

void main()
{
    float texCoordOffset = -(strokeWidth / (2.0 * gridScale));
    vec2 texCoordsTransform = (texCoords + texCoordOffset) * gridScale;
    texCoordsTransform = floor(fract(texCoordsTransform) + strokeWidth);
    float gridGradient = texCoordsTransform.x + texCoordsTransform.y;
    if (gridGradient <= 0) discard;
    FragColor = vec4(vertexColor, 1.0);
}