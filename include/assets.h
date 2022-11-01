#pragma once

#include <iostream>
#include <glm/glm.hpp>

class Object {
    public:
    unsigned int vertexData;
    unsigned int indices;
    unsigned int objectData;
    unsigned int vertexCount;
    const char* meshName;

    ~Object();
};

bool loadObject(Object* target, const char* objectLocation);

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