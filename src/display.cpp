#include <memory>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "assets.h"
#include "camera.h"
#include "solver.h"

#include "config.h"

#ifdef USE_BAKED_SHADERS
#include "baked_shaders.h"
#endif

#ifdef USE_CDDLIB

#include <quickhull/QuickHull.hpp>
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
#else
void generateSolutionObject(Object* object, const std::vector<float> vertices) {};
#endif

const glm::vec3 worldUp({0, 0, 1});

// class Display {
// private:

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
void Display::onSolutionSolved() {
    solutionObject.reset(new Object());
    generateSolutionObject(solutionObject.get(), solution.polyhedraVertices);
}

void Display::onPlaneAdded(int planeIndex) {
    visibleEquations.push_back(true);
    recalculatePlane(planeIndex);
}

void Display::onPlaneUpdated(int planeIndex) {
    recalculatePlane(planeIndex);
}

void Display::onPlaneRemoved(int planeIndex) {
    visibleEquations.erase(visibleEquations.begin() + planeIndex);
    planeTransforms.erase(planeTransforms.begin() + planeIndex);
}

//  public:

Display::Display() {
    if (!Display::planeObject) createPlaneObject();
    if (!Display::planeShader) createPlaneShader();
    this->showPlanesAtAll = true;
};

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
    if (showPlanesAtAll) {
    for (int planeIndex = 0; planeIndex < planeTransforms.size(); planeIndex++) {
        if (!visibleEquations[planeIndex]) continue;
        planeShader->setUniform("planeTransform", planeTransforms[planeIndex]);
        glDrawElements(GL_TRIANGLES, planeObject->vertexCount, GL_UNSIGNED_INT, 0);
    }
    }

    if (!this->solution.isSolved || !this->solutionObject) return;
    glBindVertexArray(this->solutionObject->objectData);
    this->planeShader->activate();
    this->planeShader->setTransform(camera->getProjection(), camera->getView());
    this->planeShader->setUniform("planeTransform", glm::mat4(1));
    glDrawElements(GL_TRIANGLES, this->solutionObject->vertexCount, GL_UNSIGNED_INT, 0);

    // TODO: render solution vector too
};

Display::~Display() {
    Display::solutionObject.reset();
    Display::planeObject.reset();
    Display::planeShader.reset();
};
// };

// class WorldGridDisplay {
//     private:
// std::shared_ptr<Object> WorldGridDisplay::gridObject;
// std::shared_ptr<Object> WorldGridDisplay::axisObject;
// std::shared_ptr<Shader> WorldGridDisplay::gridShader;
// std::shared_ptr<Shader> WorldGridDisplay::axisShader;

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
        gridShader->setUniform("gridScale", 1.0);
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