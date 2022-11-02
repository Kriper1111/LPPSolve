#include <memory>
#include <vector>
#include <list>

#include <glad/glad.h>

#include "assets.h"
#include "glm/gtx/string_cast.hpp"
#include "solver.h"

#ifdef USE_CDDLIB
#include <cdd/cdd.h>
#endif

#ifdef USE_BAKED_SHADERS
#include "baked_shaders.h"
#endif

const glm::vec3 worldUp({0, 0, 1});

// class LinearProgrammingProblemDisplay {
//  private:

std::shared_ptr<Object> LinearProgrammingProblemDisplay::planeObject;
std::shared_ptr<Shader> LinearProgrammingProblemDisplay::planeShader;

void LinearProgrammingProblemDisplay::createPlaneObject() {
    LinearProgrammingProblemDisplay::planeObject.reset(new Object());

    VertexAttributePosition vertexData[] = {
        {-1.0, -1.0, 0.0},
        {-1.0,  1.0, 0.0},
        { 1.0, -1.0, 0.0},
        { 1.0,  1.0, 0.0} 
    };

    int indices[] = {
        0, 1, 3,
        0, 3, 2
    };

    fromVertexData(planeObject.get(), vertexData, 4, indices, 6);
}

void LinearProgrammingProblemDisplay::createPlaneShader() {
    #ifdef USE_BAKED_SHADERS
    LinearProgrammingProblemDisplay::planeShader.reset(Shader::fromSource(shaders::vertex.plane_vert, shaders::fragment.default_frag));
    #else
    LinearProgrammingProblemDisplay::planeShader.reset(new Shader("assets/plane.vert", "assets/default.frag"));
    #endif
}

void LinearProgrammingProblemDisplay::recalculatePlane(int planeIndex) {
    glm::vec4 planeEquation = planeEquations[planeIndex];
    glm::vec3 planeNormal = glm::normalize(glm::vec3(planeEquation.x, planeEquation.y, planeEquation.z));

    // std::cout << "Editing: " << planeIndex << " " << glm::to_string(planeEquation) << std::endl;

    glm::mat3 planeTransform = glm::mat3(1);
    if (worldUp != planeNormal) { // e.g. we actually have to do something
        glm::vec3 right = glm::normalize(glm::cross(planeNormal, worldUp));
        glm::vec3 up = glm::normalize(glm::cross(right, planeNormal));

        planeTransform[0] = right;
        planeTransform[1] = up;
        planeTransform[2] = planeNormal;
    }

    if (planeIndex == planeTransforms.size()) { planeTransforms.push_back(planeTransform); }
    else { planeTransforms[planeIndex] = planeTransform; }

    // std::cout << "Transforms now: " << planeTransforms.size() << std::endl;

    rebindAttributes();
}
// TODO: Implement with instanced rendering
void LinearProgrammingProblemDisplay::rebindAttributes() {};
//  public:

LinearProgrammingProblemDisplay::LinearProgrammingProblemDisplay() {
    if (!LinearProgrammingProblemDisplay::planeObject) createPlaneObject();
    if (!LinearProgrammingProblemDisplay::planeShader) createPlaneShader();
};

int LinearProgrammingProblemDisplay::getEquationCount() { return planeEquations.size(); }

// DEPRECATE
int LinearProgrammingProblemDisplay::addLimitPlane(float* constraints) {
    return addLimitPlane(glm::vec4(constraints[0], constraints[1], constraints[2], constraints[3])); // BAD
}
int LinearProgrammingProblemDisplay::addLimitPlane(glm::vec4 constraints) {
    planeEquations.push_back(constraints);
    recalculatePlane(planeEquations.size() - 1);
    return planeEquations.size();
}

// Throws: std::out_of_range
glm::vec4 LinearProgrammingProblemDisplay::getLimitPlane(int planeIndex) { return planeEquations.at(planeIndex); }

// DEPRECATE
void LinearProgrammingProblemDisplay::editLimitPlane(int planeIndex, float* constraints) {
    editLimitPlane(planeIndex, glm::vec4(constraints[0], constraints[1], constraints[2], constraints[3])); // BAD
}
void LinearProgrammingProblemDisplay::editLimitPlane(int planeIndex, glm::vec4 constraints) {
    planeEquations[planeIndex] = constraints;
    recalculatePlane(planeIndex);
}

void LinearProgrammingProblemDisplay::removeLimitPlane() {
    planeEquations.pop_back();
    planeTransforms.pop_back();
    rebindAttributes();
}

void LinearProgrammingProblemDisplay::removeLimitPlane(int planeIndex) {
    if (planeIndex < 0 || planeIndex >= planeTransforms.size()) return;
    planeEquations.erase(planeEquations.begin() + planeIndex);
    planeTransforms.erase(planeTransforms.begin() + planeIndex);
    rebindAttributes();
}

bool LinearProgrammingProblemDisplay::solve() { return false; };

void LinearProgrammingProblemDisplay::renderLimitPlanes(glm::mat4 view, glm::mat4 projection) {
    if (planeTransforms.size() == 0) return;
    glBindVertexArray(planeObject->objectData);
    planeShader->activate();
    planeShader->setTransform(projection, view);
    // Instanced drawing will require:
    // * The plane transformation buffer
    //   Which should have ability to resize itself
    // * Updating this buffer with the streaming thing
    // * Re-binding attribute pointers each time the buffer is updated
    // * Just bind plane's objectData VAO and plane transformation buffer
    //   And set attributes as usual
    // glDrawElementsInstanced(GL_TRIANGLES, planeObject->vertexCount, GL_UNSIGNED_INT, 0, planeTransforms.size());
    for (glm::mat3 planeTransform : planeTransforms) {
        planeShader->setUniform("planeTransform", planeTransform);
        glDrawElements(GL_TRIANGLES, planeObject->vertexCount, GL_UNSIGNED_INT, 0);
    }
}
void LinearProgrammingProblemDisplay::renderAcceptableValues() {};
void LinearProgrammingProblemDisplay::renderSolution() {};

LinearProgrammingProblemDisplay::~LinearProgrammingProblemDisplay() {
    LinearProgrammingProblemDisplay::planeObject.reset();
    LinearProgrammingProblemDisplay::planeShader.reset();
};
// };

// class WorldGridDisplay {
//     private:
//     public:
//     WorldGridDisplay();

//     void toggleGrid();
//     void toggleWorldOrigin();

//     void zoom(float amount);
//     void render();

//     ~WorldGridDisplay();
// };