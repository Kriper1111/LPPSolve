#include "assets.h"
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "config.h"

// OBJECTS

#ifdef USE_OBJ_LOADER
#include "OBJ_Loader.h"

objl::Loader* loader = new objl::Loader();

std::shared_ptr<Object> fromWavefront(const char* objectLocation) {
    if (!loader->LoadFile(objectLocation)) {
        throw std::runtime_error("Unable to load object file");
    }

    objl::Mesh objMesh = loader->LoadedMeshes.back();
    std::vector<float> theMeshVerts;
    std::vector<GLuint> theMeshIdx;

    for (int vertNo = 0; vertNo < objMesh.Vertices.size(); vertNo ++) {
        theMeshVerts.push_back(objMesh.Vertices[vertNo].Position.X);
        theMeshVerts.push_back(objMesh.Vertices[vertNo].Position.Y);
        theMeshVerts.push_back(objMesh.Vertices[vertNo].Position.Z);
    }

    for (int vertIdx = 0; vertIdx < objMesh.Indices.size(); vertIdx ++) {
        theMeshIdx.push_back(objMesh.Indices[vertIdx]);
    }

    loader->LoadedMeshes.clear();

    std::shared_ptr<Object> object = std::make_shared<Object>();
    object->setVertexData(theMeshVerts.data(), theMeshVerts.size(), theMeshIdx.data(), theMeshIdx.size());

    return object;
}
#else
bool fromWavefront(Object* target, const char* objectLocation) { return false; }
#endif

Object::Object() {
    this->vertexCount = 0;
    this->objectData = 0;
    this->vertexData = 0;
    this->indices = 0;
}

void Object::genArrays() {
    if (this->objectData == 0) glGenVertexArrays(1, &this->objectData);
    if (this->vertexData == 0) glGenBuffers(1, &this->vertexData);
    if (this->indices == 0) glGenBuffers(1, &this->indices);
}

void Object::setVertexData(
    VertexAttributePosition* vertexData,
    size_t vertexCount,
    int* indices,
    size_t indexCount
){
    this->genArrays();
    glBindVertexArray(this->objectData);
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexData);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*vertexData) * vertexCount, vertexData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    this->vertexCount = vertexCount;

    // FIXME: Refactor this
    if (indices != nullptr) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*indices) * indexCount, indices, GL_STATIC_DRAW);
        this->vertexCount = indexCount;
    }
};

void Object::setVertexData(VertexAttributePositionUV* vertexData, size_t vertexCount, int* indices, size_t indexCount){ };

void Object::setVertexData(
    const float* vertexData,
    size_t vertexCount,
    const unsigned int* indices,
    size_t indexCount
) {
    this->genArrays();
    glBindVertexArray(this->objectData);
    glBindBuffer(GL_ARRAY_BUFFER, this->vertexData);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*vertexData) * vertexCount, vertexData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    this->vertexCount = vertexCount / 3;

    // FIXME: Refactor this
    if (indices != nullptr) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*indices) * indexCount, indices, GL_STATIC_DRAW);
        this->vertexCount = indexCount;
    }
}

void Object::bindForDraw(GLenum mode) {
    if (this->objectData == 0 || this->vertexData == 0 || this->indices == 0)
        return;
    glBindVertexArray(this->objectData);
    glDrawElements(mode, this->vertexCount, GL_UNSIGNED_INT, 0);
}

void Object::bindForDrawInstanced(GLenum mode, GLsizei count) {
    if (this->objectData == 0 || this->vertexData == 0 || this->indices == 0)
        return;
    glBindVertexArray(this->objectData);
    glDrawElementsInstanced(mode, this->vertexCount, GL_UNSIGNED_INT, 0, count);
}

void Object::bindForDraw() { this->bindForDraw(GL_TRIANGLES); }
void Object::bindForDrawInstanced(GLsizei count) { this->bindForDrawInstanced(GL_TRIANGLES, count); }

Object::~Object() {
    if (this->vertexData != 0) glDeleteBuffers(1, &this->vertexData);
    if (this->objectData != 0) glDeleteBuffers(1, &this->objectData);
    if (this->indices != 0) glDeleteBuffers(1, &this->indices);
}

// SHADERS

std::string readFileDry(const char* filename) noexcept(false) {
    std::string fileBuffer;
    std::ifstream file;
    file.exceptions( std::ifstream::failbit | std::ifstream::badbit );
    try {
        file.open(filename);
        std::stringstream reader;
        reader << file.rdbuf();
        file.close();

        fileBuffer = reader.str();
    }
    catch (std::ifstream::failure err) {
        std::cerr << "Failed to read text file " << filename << std::endl;
        throw err;
    }
    return fileBuffer;
};

char Shader::sCompileLog[512];

GLuint Shader::compileShader(const char* shaderSource, GLenum shaderType) noexcept(false) {
    GLuint shaderPtr = glCreateShader(shaderType);
    glShaderSource(shaderPtr, 1, &shaderSource, 0);
    glCompileShader(shaderPtr);

    int succ = GL_TRUE;
    glGetShaderiv(shaderPtr, GL_COMPILE_STATUS, &succ);
    if (succ != GL_TRUE) {
        glGetShaderInfoLog(shaderPtr, 512, nullptr, Shader::sCompileLog);
        std::cerr << "Failed to build a shader: " << succ << std::endl << Shader::sCompileLog << std::endl;
        std::cerr << "Shader text: " << shaderSource << std::endl;
        throw std::runtime_error("Failed to build a shader.");
    }
    return shaderPtr;
}

GLuint Shader::linkProgram(GLuint vertexStage, GLuint fragmentStage) noexcept(false) {
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexStage);
    glAttachShader(shaderProgram, fragmentStage);
    glLinkProgram(shaderProgram);
    // It's safe and even encouraged to detach and delete them
    // I doubt I'll implement caching and reusing anytime soon.
    glDetachShader(shaderProgram, vertexStage);
    glDetachShader(shaderProgram, fragmentStage);
    glDeleteShader(vertexStage);
    glDeleteShader(fragmentStage);

    int linkSuccess = GL_NO_ERROR;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_NO_ERROR) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, Shader::sCompileLog);
        std::cerr << "Failed to link a shader program: " << std::endl << Shader::sCompileLog << std::endl;
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
        throw std::runtime_error("Failed to link a shader program");
    }

    return shaderProgram;
}

Shader::Shader(char const* vertexShaderPath, char const* fragmentShaderPath) {
    std::string vertexShaderText = readFileDry(vertexShaderPath);
    std::string fragmentShaderText = readFileDry(fragmentShaderPath);

    GLuint vertexShader = Shader::compileShader(vertexShaderText.c_str(), GL_VERTEX_SHADER);
    GLuint fragmentShader = Shader::compileShader(fragmentShaderText.c_str(), GL_FRAGMENT_SHADER);
    this->pShaderProgram = Shader::linkProgram(vertexShader, fragmentShader);
}

Shader::Shader(GLuint programId) {
    this->pShaderProgram = programId;
}

Shader* Shader::fromSource(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexStage = Shader::compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentStage = Shader::compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    GLuint shaderProgram = Shader::linkProgram(vertexStage, fragmentStage);

    return new Shader(shaderProgram);
}

Shader::~Shader() { if (this->pShaderProgram != 0) glDeleteProgram(this->pShaderProgram); }

void Shader::activate() { glUseProgram(this->pShaderProgram); }

void Shader::setTransform(glm::mat4 projection, glm::mat4 view) {
    glUniformMatrix4fv(glGetUniformLocation(this->pShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(this->pShaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
}

void Shader::setUniform(const char *name, int uniformValue) {
    glUniform1i(glGetUniformLocation(this->pShaderProgram, name), uniformValue);
}

void Shader::setUniform(const char*name, float uniformValue) {
    glUniform1f(glGetUniformLocation(this->pShaderProgram, name), uniformValue);
}

void Shader::setUniform(const char*name, double uniformValue) {
    glUniform1f(glGetUniformLocation(this->pShaderProgram, name), uniformValue);
}

void Shader::setUniform(const char*name, glm::vec2 uniformValue) {
    glUniform2f(glGetUniformLocation(this->pShaderProgram, name), uniformValue.x, uniformValue.y);
}

void Shader::setUniform(const char*name, glm::vec3 uniformValue) {
    glUniform3f(glGetUniformLocation(this->pShaderProgram, name), uniformValue.x, uniformValue.y, uniformValue.z);
}

void Shader::setUniform(const char*name, glm::vec4 uniformValue) {
    glUniform4f(glGetUniformLocation(this->pShaderProgram, name), uniformValue.x, uniformValue.y, uniformValue.z, uniformValue.w);
}

void Shader::setUniform(const char*name, glm::mat2 uniformValue) {
    glUniformMatrix2fv(glGetUniformLocation(this->pShaderProgram, name), 1, GL_FALSE, &uniformValue[0][0]);
}

void Shader::setUniform(const char*name, glm::mat3 uniformValue) {
    glUniformMatrix3fv(glGetUniformLocation(this->pShaderProgram, name), 1, GL_FALSE, &uniformValue[0][0]);
}

void Shader::setUniform(const char*name, glm::mat4 uniformValue) {
    glUniformMatrix4fv(glGetUniformLocation(this->pShaderProgram, name), 1, GL_FALSE, &uniformValue[0][0]);
}