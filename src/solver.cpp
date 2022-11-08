#include <memory>
#include <vector>
#include <list>

#include <glad/glad.h>

#include "assets.h"
#include "camera.h"
#include "glm/gtx/string_cast.hpp"
#include "solver.h"

#ifdef USE_CDDLIB
#define REFLECT(var) #var
#include <cdd/setoper.h>
#include <cdd/cdd.h>

#include <quickhull/QuickHull.hpp>
#endif

#ifdef USE_BAKED_SHADERS
#include "baked_shaders.h"
#endif

const glm::vec3 worldUp({0, 0, 1});

#ifdef USE_CDDLIB
// XXX: This code is ugly and could at least be baked in
const char* reflect_dd_error(dd_ErrorType error) {
    switch (error)
    {
    case dd_DimensionTooLarge: return REFLECT(dd_DimensionTooLarge);
    case dd_ImproperInputFormat: return REFLECT(dd_ImproperInputFormat);
    case dd_NegativeMatrixSize: return REFLECT(dd_NegativeMatrixSize);
    case dd_EmptyVrepresentation: return REFLECT(dd_EmptyVrepresentation);
    case dd_EmptyHrepresentation: return REFLECT(dd_EmptyHrepresentation);
    case dd_EmptyRepresentation: return REFLECT(dd_EmptyRepresentation);
    case dd_IFileNotFound: return REFLECT(dd_IFileNotFound);
    case dd_OFileNotOpen: return REFLECT(dd_OFileNotOpen);
    case dd_NoLPObjective: return REFLECT(dd_NoLPObjective);
    case dd_NoRealNumberSupport: return REFLECT(dd_NoRealNumberSupport);
    case dd_NotAvailForH: return REFLECT(dd_NotAvailForH);
    case dd_NotAvailForV: return REFLECT(dd_NotAvailForV);
    case dd_CannotHandleLinearity: return REFLECT(dd_CannotHandleLinearity);
    case dd_RowIndexOutOfRange: return REFLECT(dd_RowIndexOutOfRange);
    case dd_ColIndexOutOfRange: return REFLECT(dd_ColIndexOutOfRange);
    case dd_LPCycling: return REFLECT(dd_LPCycling);
    case dd_NumericallyInconsistent: return REFLECT(dd_NumericallyInconsistent);
    case dd_NoError: return REFLECT(dd_NoError);
    default: return "Unknown dd_ErrorType";
    }
}

// @throws std::runtime_error if there's a dd error
void throw_dd_error(dd_ErrorType error) {
    if (error != dd_NoError) {
        throw std::runtime_error(reflect_dd_error(error));
    }
}

glm::vec4 createVector(dd_Arow dd_vector, int vector_length) {
    glm::vec4 vector(0);
    for (int index = 0; index < vector_length && index < 4; index++) {
        vector[index] = dd_vector[index][0];
    }
    return vector;
}

std::vector<float> getVertices(dd_MatrixPtr vform) {
    std::vector<float> vertices;
    for (int row = 0; row < vform->rowsize; row++) {
        if (vform->matrix[row][0][0] == 0) continue;
        vertices.push_back(vform->matrix[row][1][0]);
        vertices.push_back(vform->matrix[row][2][0]);
        vertices.push_back(vform->matrix[row][3][0]);
    }
    return vertices;
}

std::vector<size_t> getIndices(const std::vector<float> vertices) {
    quickhull::QuickHull<float> qh;
    auto convexHull = qh.getConvexHull(vertices.data(), vertices.size(), true, false);
    return convexHull.getIndexBuffer();
}

void generateSolutionObject(Object* object, const std::vector<float> vertices) {
    quickhull::QuickHull<float> qh;

    auto convexHull = qh.getConvexHull(vertices.data(), vertices.size() / 3, true, true);
    auto indexBuffer = convexHull.getIndexBuffer();

    // Cast to <int> because for some reason OpenGL doesn't like anything other than
    // *(u)int* in its index buffer
    std::vector<int> indices(indexBuffer.begin(), indexBuffer.end());

    fromVertexData(object, vertices.data(), vertices.size(), indices.data(), (int)indexBuffer.size());
}

#endif // USE_CDDLIB

// class LinearProgrammingProblem {
// protected:
void LinearProgrammingProblem::collectPointless() {
    // It never gets better, does it?
    // I hope they're sorted
    for (const int index : this->pointlessEquations)
        this->removeLimitPlane(index);
    this->pointlessEquations.clear();
}

//  public:
LinearProgrammingProblem::LinearProgrammingProblem() {};
LinearProgrammingProblem::~LinearProgrammingProblem() {};

int LinearProgrammingProblem::getEquationCount() {
    this->collectPointless();
    return planeEquations.size();
}

int LinearProgrammingProblem::addLimitPlane(glm::vec4 constraints) {
    if (constraints.x != 0 || constraints.y != 0 || constraints.z != 0 || constraints.w != 0) {
        planeEquations.push_back(constraints);
        // visibleEquations.push_back(true);
        recalculatePlane(planeEquations.size() - 1);
    }
    return planeEquations.size();
}

// @throws: std::out_of_range
glm::vec4 LinearProgrammingProblem::getLimitPlane(int planeIndex) { return planeEquations.at(planeIndex); }

void LinearProgrammingProblem::editLimitPlane(int planeIndex, glm::vec4 constraints) {
    planeEquations[planeIndex] = constraints;
    recalculatePlane(planeIndex);
    if (constraints.x == 0 && constraints.y == 0 && constraints.z == 0 && constraints.w == 0) {
        this->pointlessEquations.push_back(planeIndex);
    }
}

void LinearProgrammingProblem::removeLimitPlane() {
    planeEquations.pop_back();
    // visibleEquations.pop_back();
}
void LinearProgrammingProblem::removeLimitPlane(int planeIndex) {
    if (planeIndex < 0 || planeIndex >= planeEquations.size()) return;
    planeEquations.erase(planeEquations.begin() + planeIndex);
    // visibleEquations.erase(visibleEquations.begin() + planeIndex);
}

template <typename dd_Type>
using dd_unique_ptr = std::unique_ptr<dd_Type, void(*)(dd_Type*)>;

// Solves the given LPP and return boolean if a solution was found
// @throws std::runtime_error if there's a dd error
void LinearProgrammingProblem::solve() {
    // Yes we use #ifdef and I know it's bad, but I have to build it somehow on Windows first.
    #ifdef USE_CDDLIB
    // XXX: This is less likely to throw a segfault than plainly deleting them, but who knows
    //      This approach relies on RAII to deinit the values. The question is whether I can
    //      deinit them after I dd_free_global_constants();
    dd_unique_ptr<dd_LPType> linearProgrammingProblem(nullptr, dd_FreeLPData);
    dd_unique_ptr<dd_MatrixType> constraintMatrix(nullptr, dd_FreeMatrix);
    dd_unique_ptr<dd_MatrixType> verticesMatrix(nullptr, dd_FreeMatrix);
    dd_unique_ptr<dd_PolyhedraType> polyhedra(nullptr, dd_FreePolyhedra);
    dd_ErrorType error;
    try {
    dd_set_global_constants();

    solution = Solution();

    constraintMatrix.reset(dd_CreateMatrix(3 + this->planeEquations.size(), 4));

    int row;
    for (row = 0; row < planeEquations.size(); row++) {
        // Ah yes I love doing stuff this way. Just can't get enough of it.
        // *sarcarsm please don't judge*
        /** XXX: cddlib expects us to provide the constraints in a different form
         * it expects the form of:
         * B A1 A2 A3 >= 0
         * but we collect them in form of
         * A1 A2 A3 <= B
         * so we have to multiply A's (planeEquation.xyz) by -1
         * And shift B (planeEquation.w) to the first column.
         */
        const auto planeEquation = planeEquations.at(row);
        dd_set_d(constraintMatrix->matrix[row][0],  planeEquation.w);
        dd_set_d(constraintMatrix->matrix[row][1], -planeEquation.x);
        dd_set_d(constraintMatrix->matrix[row][2], -planeEquation.y);
        dd_set_d(constraintMatrix->matrix[row][3], -planeEquation.z);
    }
    for (int diag = 0; diag < 3; diag++) {
        // -This way we set the x1, x2, x3 >= 0 condition
        // -Other form is
        // dd_set_d(constraintMatrix->matrix[row + 0][1], 1.0);
        // dd_set_d(constraintMatrix->matrix[row + 1][2], 1.0);
        // dd_set_d(constraintMatrix->matrix[row + 2][3], 1.0);
        //  without the loop
        dd_set_d(constraintMatrix->matrix[row + diag][diag + 1], 1.0);
    }

    dd_set_d(constraintMatrix->rowvec[0],  objectiveFunction.w);
    dd_set_d(constraintMatrix->rowvec[1], -objectiveFunction.x);
    dd_set_d(constraintMatrix->rowvec[2], -objectiveFunction.y);
    dd_set_d(constraintMatrix->rowvec[3], -objectiveFunction.z);

    constraintMatrix->representation = dd_Inequality;
    constraintMatrix->objective = this->doMinimize ? dd_LPmin : dd_LPmax;

    linearProgrammingProblem.reset(dd_Matrix2LP(constraintMatrix.get(), &error));
    throw_dd_error(error);
    polyhedra.reset(dd_DDMatrix2Poly(constraintMatrix.get(), &error));
    throw_dd_error(error);
    dd_LPSolve(linearProgrammingProblem.get(), dd_DualSimplex, &error);
    throw_dd_error(error);

    verticesMatrix.reset(dd_CopyGenerators(polyhedra.get()));

    solution.isSolved = true;
    solution.optimalValue = linearProgrammingProblem->optvalue[0];
    solution.optimalVector = createVector(linearProgrammingProblem->sol, linearProgrammingProblem->d);
    solution.didMinimize = linearProgrammingProblem->objective == dd_LPmin;
    solution.polyhedraVertices = getVertices(verticesMatrix.get());
    onSolutionSolved();

    } catch (std::runtime_error &dd_error) {
        dd_free_global_constants();
        throw dd_error;
    }
    dd_free_global_constants();
    #endif
};

const LinearProgrammingProblem::Solution* LinearProgrammingProblem::getSolution() {
    return &this->solution;
}

/**
 * FIXME: This whole thing isn't thread-safe. At all. Which is a candidate
 * to a change, don't want to hang rendering for too long with calculations.
*/
// class Display {
// private:

std::shared_ptr<Object> Display::planeObject;
std::shared_ptr<Shader> Display::planeShader;

void Display::createPlaneObject() {
    Display::planeObject.reset(new Object());

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

void Display::createPlaneShader() {
    #ifdef USE_BAKED_SHADERS
    Display::planeShader.reset(Shader::fromSource(shaders::vertex.plane_vert, shaders::fragment.default_frag));
    #else
    Display::planeShader.reset(new Shader("assets/plane.vert", "assets/default.frag"));
    #endif
}

void Display::recalculatePlane(int planeIndex) {
    glm::vec4 planeEquation = planeEquations[planeIndex];
    glm::vec3 planeNormal = glm::normalize(glm::vec3(planeEquation.x, planeEquation.y, planeEquation.z));

    // std::cout << "Editing: " << planeIndex << " " << glm::to_string(planeEquation) << std::endl;

    // float lengthSquared = glm::pow(planeEquation.x, 2) +
    //                       glm::pow(planeEquation.y, 2) +
    //                       glm::pow(planeEquation.z, 2);
    // float lengthDoubled = glm::length(glm::vec3(planeEquation.x, planeEquation.y, planeEquation.z)) * 2;
    float distanceToLine = planeEquation.w / glm::length(glm::vec3(planeEquation.x, planeEquation.y, planeEquation.z));
    // So I was kind of correct with the guesses.. Kind of.
    // Really shows how much easier it is when you actually think before writing code and
    //      making any assumptions based off of a simplifed example in Blender.
    // It's worse than I thought
    // It's trigonometry (oh no)
    // Update: I'm extra stupid right now. It's just basic vector things.

    glm::mat4 planeTransform = glm::mat4(1);
    if (worldUp == glm::abs(planeNormal)) {
        /** HACK: This is done to properly orient the plane when it's aligned
         *          with the world axis -- in this case it's hardcoded to Z+
         *        We modify *up* value instead of *right*
         *        But we're better off merging it with the default workflow
         *        E.g. have a default *right* vector and fall back to that
         *          if planeNormal and worldUp align. *how would we check it though*
         *        Probably via the abs method, can't think of anything good right now
         */
        planeTransform[1] = { 0, -planeNormal.z, 0, 0 };
    } else {
        glm::vec3 right = glm::normalize(glm::cross(planeNormal, worldUp));
        glm::vec3 up = glm::normalize(glm::cross(right, planeNormal));

        planeTransform[0] = { right, 0 };
        planeTransform[1] = { up, 0 };
        planeTransform[2] = { planeNormal, 0 };
    }

    planeTransform[3] = { planeNormal * distanceToLine, 1 };

    planeTransform = glm::scale(planeTransform, { 10.0, 10.0, 10.0 });

    if (planeIndex == planeTransforms.size()) { planeTransforms.push_back(planeTransform); }
    else { planeTransforms[planeIndex] = planeTransform; }

    rebindAttributes();
}
// TODO: Implement with instanced rendering
void Display::rebindAttributes() {};
//  public:

Display::Display() {
    if (!Display::planeObject) createPlaneObject();
    if (!Display::planeShader) createPlaneShader();
};

int Display::getEquationCount() {
    this->collectPointless();
    return LinearProgrammingProblem::getEquationCount();
}

// int Display::addLimitPlane(glm::vec4 constraints) {
//     if (constraints.x != 0 || constraints.y != 0 || constraints.z != 0 || constraints.w != 0) {
//         planeEquations.push_back(constraints);
//         visibleEquations.push_back(true);
//         recalculatePlane(planeEquations.size() - 1);
//     }
//     return planeEquations.size();
// }

// // DEPRECATE
// void Display::editLimitPlane(int planeIndex, float* constraints) {
//     editLimitPlane(planeIndex, glm::vec4(constraints[0], constraints[1], constraints[2], constraints[3])); // BAD
// }
// void Display::editLimitPlane(int planeIndex, glm::vec4 constraints) {
//     planeEquations[planeIndex] = constraints;
//     recalculatePlane(planeIndex);
//     if (constraints.x == 0 && constraints.y == 0 && constraints.z == 0 && constraints.w == 0) {
//         this->pointlessEquations.push_back(planeIndex);
//     }
// }

// void Display::removeLimitPlane() {
//     planeEquations.pop_back();
//     planeTransforms.pop_back();
//     visibleEquations.pop_back();
//     rebindAttributes();
// }
// void Display::removeLimitPlane(int planeIndex) {
//     if (planeIndex < 0 || planeIndex >= planeTransforms.size()) return;
//     planeEquations.erase(planeEquations.begin() + planeIndex);
//     planeTransforms.erase(planeTransforms.begin() + planeIndex);
//     visibleEquations.erase(visibleEquations.begin() + planeIndex);
//     rebindAttributes();
// }

void Display::onSolutionSolved() {
    solutionObject.reset(new Object());
    generateSolutionObject(solutionObject.get(), solution.polyhedraVertices);
}

void Display::render(Camera* camera) {
    if (planeTransforms.size() == 0) return;
    glBindVertexArray(planeObject->objectData);
    planeShader->activate();
    planeShader->setTransform(camera->getProjection(), camera->getView());
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
    for (int planeIndex = 0; planeIndex < planeTransforms.size(); planeIndex++) {
        // if (!visibleEquations[planeIndex]) continue;
        planeShader->setUniform("planeTransform", planeTransforms[planeIndex]);
        glDrawElements(GL_TRIANGLES, planeObject->vertexCount, GL_UNSIGNED_INT, 0);
    }

    if (!this->solution.isSolved || !this->solutionObject) return;
    glBindVertexArray(this->solutionObject->objectData);
    this->planeShader->activate();
    this->planeShader->setTransform(camera->getProjection(), camera->getView());
    this->planeShader->setUniform("planeTransform", glm::mat4(1));
    glDrawElements(GL_TRIANGLES, this->solutionObject->vertexCount, GL_UNSIGNED_INT, 0);

    // XXX: render solution vector too
};

Display::~Display() {
    Display::solutionObject.reset();
    Display::planeObject.reset();
    Display::planeShader.reset();
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

void WorldGridDisplay::zoomGrid(float amount) {
    zoomScale += amount;
    while (zoomScale > 2.0) zoomScale -= 2.0;
    while (zoomScale < 1.0) zoomScale += 1.0;
}

void WorldGridDisplay::render(glm::mat4 view, glm::mat4 projection) {
    if (gridEnabled) {
        gridShader->activate();
        gridShader->setTransform(projection, view);

        glBindVertexArray(gridObject->objectData);
        gridShader->setUniform("gridScale", zoomScale / 2.0);
        glDrawElements(GL_LINES, gridObject->vertexCount, GL_UNSIGNED_INT, 0);
        gridShader->setUniform("gridScale", zoomScale);
        glDrawElements(GL_LINES, gridObject->vertexCount, GL_UNSIGNED_INT, 0);
        gridShader->setUniform("gridScale", zoomScale * 2.0);
        glDrawElements(GL_LINES, gridObject->vertexCount, GL_UNSIGNED_INT, 0);
    }
    if (axisEnabled) {
        gridShader->activate();
        gridShader->setTransform(projection, view);
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