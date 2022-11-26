#pragma once

#include <memory>
#include <glm/glm.hpp>

struct VertexAttributePosition {
    float position[3];
};
struct VertexAttributePositionUV {
    float position[3];
    float uv[2];
};

class Object {
    private:
    unsigned int vertexData;
    unsigned int indices;
    unsigned int objectData;
    unsigned int vertexCount;
    const char* meshName;

    void genArrays();

    public:
    Object();

    void setVertexData(const float* vertexData, size_t vertexCount, const unsigned int* indices, size_t indexCount);
    void setVertexData(VertexAttributePosition* vertexData, size_t vertexCount, int* indices, size_t indexCount);
    void setVertexData(VertexAttributePositionUV* vertexData, size_t vertexCount, int* indices, size_t indexCount);
    void setFaceData(); // Unimplemented

    static std::shared_ptr<Object> fromWavefront(const char* objectLocation);

    void bindForDraw(unsigned int mode) const;
    void bindForDraw() const;

    void bindForDrawInstanced(unsigned int mode, int count) const;
    void bindForDrawInstanced(int count) const;

    void bindForDrawSlice(unsigned int mode, size_t offset, int vertices) const;
    void bindForDrawSlice(size_t offset, int vertices) const;

    ~Object();
};

class Shader {
    private:
    unsigned int pShaderProgram;
    Shader(unsigned int programId);

    static std::unique_ptr<char>sCompileLog;
    static unsigned int compileShader(const char* shaderSource, unsigned int shaderType);
    static unsigned int linkProgram(unsigned int vertexStage, unsigned int fragmentStage) noexcept(false);
    public:
    Shader(char const* vertexShaderPath, char const* fragmentShaderPath);

    static Shader* fromSource(const char* vertexSource, const char* fragmentSource);

    void activate() const;
    void setTransform(glm::mat4 projectionMatrix, glm::mat4 viewMatrix) const;

    void setUniform(const char* name, int value) const;
    void setUniform(const char* name, float value) const;
    void setUniform(const char* name, double value) const;

    void setUniform(const char* name, glm::vec2 value) const;
    void setUniform(const char* name, glm::vec3 value) const;
    void setUniform(const char* name, glm::vec4 value) const;

    void setUniform(const char* name, glm::mat2 value) const;
    void setUniform(const char* name, glm::mat3 value) const;
    void setUniform(const char* name, glm::mat4 value) const;

    ~Shader();
};
