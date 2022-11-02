#pragma once

#include <iostream>
#include <glm/glm.hpp>

// namespace kegl {

struct VertexAttributePosition {
    float position[3];
};
struct VertexAttributePositionUV {
    float position[3];
    float uv[2];
};

class Object {
    public:
    unsigned int vertexData;
    unsigned int indices;
    unsigned int objectData;
    unsigned int vertexCount;
    const char* meshName;

    ~Object();
};

bool fromVertexData(Object* object, VertexAttributePosition* vertexData, int vertexCount, int* indices, int indexCount);
bool fromVertexData(Object* object, VertexAttributePositionUV* vertexData, int vertexCount, int* indices, int indexCount);
bool fromWavefront(Object* target, const char* objectLocation);

class Shader {
    private:
    unsigned int pShaderProgram;
    Shader(unsigned int programId);

    static char sCompileLog[512];
    static unsigned int compileShader(const char*, unsigned int);
    static unsigned int linkProgram(unsigned int, unsigned int);
    public:
    Shader(char const* vertexShaderPath, char const* fragmentShaderPath);
    ~Shader();

    static Shader* fromSource(const char* vertexSource, const char* fragmentSource);

    int checkCompileStatus();
    void activate();
    void setTransform(glm::mat4 projectionMatrix, glm::mat4 viewMatrix);
    template <class T>
    void setUniform(const char* name, T value);
};

// } // namespace: kegl