#pragma once

namespace shaders {
    struct {
        const char* generic = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec2 aUV;

            out vec2 texCoords;

            uniform mat4 projection = mat4(1.0);
            uniform mat4 view = mat4(1.0);

            void main()
            {
                texCoords = aUV;
                mat4 transform = mat4(1.0);
                gl_Position = projection * view * transform * vec4(aPos, 1.0);
            }
        )";
        
        const char* font = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec2 aUV;

            uniform mat4 projection;
            uniform mat4 view;

            out vec2 texCoords;

            void main() {
                texCoords = aUV;
                gl_Position = projection * view * vec4(aPos.x, aPos.y, 0.0, 1.0);
            }
        )";
    } vertex;

    struct {
        const char* generic = R"(
            #version 330 core
            out vec4 FragColor;

            in vec2 texCoords;

            uniform sampler2D imageTexture;
            uniform vec3 vertexColor = vec3(1.0, 1.0, 1.0);

            void main()
            {
                FragColor = texture(imageTexture, texCoords) * vec4(vertexColor, 1.0);
            }
        )";

        const char* font = R"(
            #version 330 core
            out vec4 FragColor;

            in vec2 texCoords;

            uniform sampler2D fragTexture;
            uniform vec3 fontColor = vec3(1.0, 1.0, 1.0);

            void main() {
                vec4 color = texture(fragTexture, texCoords).rrrr * vec4(fontColor, 1.0);
                if (color.a < 0.5) discard;
                FragColor = color;
            }
        )";
    } fragment;
};