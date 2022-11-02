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
    /** TODO: Instanced rendering of plane limits
     * This will require:
     * - The plane transformation buffer [GL]
     *   All the mat3s are collected there
     * - Said buffer to be resizeable and dynamically updateable
     *   Not every frame, but could be often
     * - Meaning we should somehow rebind it
     *   OpenGL allows for linking *side*("unrelated") buffers to the VAO
     *   So we could have the vertex data buffer with our glorious four vertices
     *   And another one, with the matrix transforms
     * - Passing matrix transforms via vertex attributes
     *   Which isn't that bad, as they're involved in vertex stage, which uses them either way
     * 
     * I'm mostly concerned about re-creating the MBO (matrix buffer object)
     * Like, do we have to re-link it to the VAO? What happens to the old link?
     * Do we have to re-link the whole object?
     */
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
std::shared_ptr<Object> WorldGridDisplay::gridObject;
std::shared_ptr<Object> WorldGridDisplay::axisObject;
std::shared_ptr<Shader> WorldGridDisplay::gridShader;
std::shared_ptr<Shader> WorldGridDisplay::axisShader;

void WorldGridDisplay::createObjects() {
    WorldGridDisplay::gridObject.reset(new Object());

    // 11 vertices per side, 4 sides
    // Yes those 4 extra could get reduced
    // But, for your consideration: I can't be assed
    // XXX: Let me know if precisely *this* allocation will be the cause of an OOM
    VertexAttributePosition gridVertexData[44];
    int gridIndices[44];
    for (int x = 0; x <= 10; x++) {
        gridIndices[2 * x] = x;
        gridIndices[2 * x + 1] = 11 + x;
        gridVertexData[ 0 + x] = { x - 5.0f, -5.0f, 0.0f };
        gridVertexData[11 + x] = { x - 5.0f,  5.0f, 0.0f };
    }
    for (int y = 0; y <= 10; y++) {
        gridIndices[22 + (2 * y)] = 22 + y;
        gridIndices[22 + (2 * y) + 1] = 33 + y;
        gridVertexData[22 + y] = { -5.0f, y - 5.0f, 0.0f };
        gridVertexData[33 + y] = {  5.0f, y - 5.0f, 0.0f };
    }

    fromVertexData(WorldGridDisplay::gridObject.get(), gridVertexData, 44, gridIndices, 44);

    /** FIXME: Compact either manual object creation or shaders into less *objects*
     * Right now I have to use two different shaders for basically the same generic object
     * So I would avoid creating and mainly repeating the same operation with loading
     * VertexAttributePositionUV
     * 
     * This needs to be redone, probably via multiple data buffers or appending to the end of one
     * while collecting the offsets/strides from previous steps
     * so .addVertexAttribute3D() -> strides: [3]
     *    .addVertexAttribute2D() -> strides: [3, 2]
     *    .addVertexAttribute3D() -> strides: [3, 2, 3]
     * and during finalization phase, iterate over $strides and do something like
     *    glVertexAttribPointer(i, stride, attribute_type, normalized, $stride * sizeof(attribute_type), (void *) rolling_sum_of_sizes );
     * This does have flaws, already visible. Mainly, attribute_type is not considered, and it's very important
     */
    WorldGridDisplay::axisObject.reset(new Object());
    VertexAttributePosition axisVertexData[] = {
        {0, 0, 0},
        {2, 0, 0},
        {0, 2, 0},
        {0, 0, 2}
    };
    int axisIndices[8] = {
        0, 1,
        0, 2,
        0, 3
    };

    fromVertexData(WorldGridDisplay::axisObject.get(), axisVertexData, 4, axisIndices, 6);
}

void WorldGridDisplay::createShaders() {
    #ifdef USE_BAKED_SHADERS
    WorldGridDisplay::gridShader.reset(Shader::fromSource(shaders::vertex.grid_vert, shaders::fragment.grid_frag));
    // WorldGridDisplay::axisShader.reset(Shader::fromSource(shaders::vertex.default_vert, shaders::fragment.default_frag));
    #else
    WorldGridDisplay::gridShader.reset(new Shader("assets/grid.vert", "assets/grid.frag"));
    // WorldGridDisplay::axisShader.reset(new Shader("assets/default.vert", "assets/default.frag"));
    #endif
}

// class WorldGridDisplay
//     public:

WorldGridDisplay::WorldGridDisplay() {
    if (!WorldGridDisplay::gridObject || !WorldGridDisplay::axisObject) WorldGridDisplay::createObjects();
    if (!WorldGridDisplay::gridShader || !WorldGridDisplay::axisShader) WorldGridDisplay::createShaders();
}

void WorldGridDisplay::render(glm::mat4 view, glm::mat4 projection) {
    if (gridEnabled) {
        gridShader->activate();
        gridShader->setTransform(projection, view);
        gridShader->setUniform("gridScale", zoomScale);

        glBindVertexArray(gridObject->objectData);
        // XXX: count in scale and some kind of threshold system
        glDrawElements(GL_LINES, gridObject->vertexCount, GL_UNSIGNED_INT, 0);
    }
    if (axisEnabled) {
        gridShader->activate();
        gridShader->setTransform(projection, view);
        // axisShader->activate();
        // axisShader->setTransform(projection, view);
        glBindVertexArray(axisObject->objectData);
        glDrawElements(GL_LINES, axisObject->vertexCount, GL_UNSIGNED_INT, 0);
    }
}

WorldGridDisplay::~WorldGridDisplay() {
    WorldGridDisplay::gridObject.reset();
    WorldGridDisplay::axisObject.reset();
    WorldGridDisplay::gridShader.reset();
    WorldGridDisplay::axisShader.reset();
}
// };