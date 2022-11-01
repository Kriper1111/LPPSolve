#include <assets.h>
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "OBJ_Loader.h"

objl::Loader* loader = new objl::Loader();

/**
 * XXX:
 * This method now throws, so it might get a bit fucky-wucky™ with opengl during initialization.
 */
std::string readFileDry(const char* filename) {
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
        std::cout << "Failed to read text file " << filename << std::endl;
        throw err;
    }
    return fileBuffer;
};

Object::~Object() {
    glDeleteBuffers(1, &this->vertexData);
    glDeleteBuffers(1, &this->indices);
    glDeleteBuffers(1, &this->objectData);
}

bool loadObject(Object* target, const char* objectLocation) {
    if (!loader->LoadFile(objectLocation)) {
        return false;
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

    target->vertexCount = objMesh.Vertices.size();
    glGenVertexArrays(1, &target->objectData);

    glBindVertexArray(target->objectData);
    glGenBuffers(1, &target->indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, target->indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, theMeshIdx.size() * sizeof(GLuint), theMeshIdx.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &target->vertexData);
    glBindBuffer(GL_ARRAY_BUFFER, target->vertexData);
    glBufferData(GL_ARRAY_BUFFER, theMeshVerts.size() * sizeof(float), theMeshVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    return true;
}

char Shader::sCompileLog[512];

GLuint Shader::compileShader(const char* text, GLenum shaderType) {
    GLuint shaderPtr = glCreateShader(shaderType);
    glShaderSource(shaderPtr, 1, &text, 0);
    glCompileShader(shaderPtr);

    int succ;
    glGetShaderiv(shaderPtr, GL_COMPILE_STATUS, &succ);
    if (!succ) {
        glGetShaderInfoLog(shaderPtr, 512, NULL, Shader::sCompileLog);
        std::cout << "Failed to build a shader: " << std::endl << Shader::sCompileLog << std::endl;
        std::cout << "Shader text: " << text << std::endl;
        return -1;
    }
    return shaderPtr;
}

GLuint Shader::linkProgram(GLuint vertexStage, GLuint fragmentStage) {
    if (vertexStage == -1 || fragmentStage == -1) {
        std::cout << "Unable to link a shader program: Failed to build shaders" << std::endl;
        return 0;
    }
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexStage);
    glAttachShader(shaderProgram, fragmentStage);
    glLinkProgram(shaderProgram);

    int linkSuccess;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkSuccess);
    if (!linkSuccess) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, Shader::sCompileLog);
        std::cout << "Failed to link a shader program: " << std::endl << Shader::sCompileLog << std::endl;
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }

    return shaderProgram;
}

Shader::Shader(char const* vertexShaderLocation, char const* fragmentShaderLocation) {
    std::string vertexShaderText = readFileDry(vertexShaderLocation);
    std::string fragmentShaderText = readFileDry(fragmentShaderLocation);

    unsigned int vertexShader = this->compileShader(vertexShaderText.c_str(), GL_VERTEX_SHADER);
    unsigned int fragmentShader = this->compileShader(fragmentShaderText.c_str(), GL_FRAGMENT_SHADER);
    this->pShaderProgram = this->linkProgram(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::Shader(GLuint programId) {
    this->pShaderProgram = programId;
}

Shader* Shader::fromSource(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexStage = Shader::compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentStage = Shader::compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    GLuint shaderProgram = Shader::linkProgram(vertexStage, fragmentStage);
    glDeleteShader(vertexStage);
    glDeleteShader(fragmentStage);

    return new Shader(shaderProgram);
}

Shader::~Shader() { glDeleteProgram(this->pShaderProgram); }

int Shader::checkCompileStatus() { return this->pShaderProgram != 0; }
void Shader::activate() { glUseProgram(this->pShaderProgram); }

void Shader::setTransform(glm::mat4 projection, glm::mat4 view) {
    glUniformMatrix4fv(glGetUniformLocation(this->pShaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(this->pShaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
}

template <>
void Shader::setUniform<int>(const char *name, int uniformValue) {
    glUniform1i(glGetUniformLocation(this->pShaderProgram, name), uniformValue);
}

template <>
void Shader::setUniform<float>(const char*name, float uniformValue) { glUniform1f(glGetUniformLocation(this->pShaderProgram, name), uniformValue); }

template<>
void Shader::setUniform<glm::vec3>(const char*name, glm::vec3 uniformValue) { glUniform3f(glGetUniformLocation(this->pShaderProgram, name), uniformValue.x, uniformValue.y, uniformValue.z); }

template<>
void Shader::setUniform<glm::mat4>(const char*name, glm::mat4 uniformValue) { glUniformMatrix4fv(glGetUniformLocation(this->pShaderProgram, name), 1, GL_FALSE, &uniformValue[0][0]); }

template<>
void Shader::setUniform<glm::mat3>(const char*name, glm::mat3 uniformValue) { glUniformMatrix3fv(glGetUniformLocation(this->pShaderProgram, name), 1, GL_FALSE, &uniformValue[0][0]); }