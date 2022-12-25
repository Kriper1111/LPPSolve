#include <memory>
#include <vector>
#include <algorithm>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define DISPLAY_IMPL
#include "assets.h"
#include "camera.h"
#include "solver.h"

#include "config.h"

#ifdef USE_BAKED_SHADERS
#include "baked_shaders.h"
#endif

#ifdef USE_CDDLIB

#include <quickhull/QuickHull.hpp>
std::vector<size_t> getIndices(const std::vector<float>& vertices) {
    quickhull::QuickHull<float> qh;
    auto convexHull = qh.getConvexHull(vertices.data(), vertices.size(), true, false);
    return convexHull.getIndexBuffer();
}

void generateSolutionObject(Object* object, const std::vector<float>& vertices) {
    quickhull::QuickHull<float> qh;

    auto convexHull = qh.getConvexHull(vertices.data(), vertices.size() / 3, true, true);
    auto indexBuffer = convexHull.getIndexBuffer();

    // Cast to <int> because for some reason OpenGL doesn't like anything other than
    // *(u)int* in its index buffer
    std::vector<unsigned int> indices(indexBuffer.begin(), indexBuffer.end());

    object->setVertexData(vertices.data(), vertices.size(), indices.data(), indexBuffer.size());
}

void generateSolutionWireframe(Object* object, const std::vector<float>& vertices, const std::vector<std::vector<int>>& adjacency) {
    std::vector<unsigned int> indices;
    int vertexCount = vertices.size() / 3;

    #ifdef DEBUG
    std::cout << "###############################\n";
    std::cout << "# BEGIN WIREFRAME GENERATION\n";
    std::cout << "###############################\n";

    std::cout << "Total vertices: " << vertexCount << '\n';
    std::cout << "Total adjacency entries: " << adjacency.size() << '\n';

    std::cout << "The pairing output will follow.\n";
    #endif

    for (int vertex_a = 0; vertex_a < adjacency.size(); vertex_a++) {
        const auto &adjacent = adjacency[vertex_a];
        for (int vertex_b : adjacent) {
            vertex_b -= 1;

            #ifdef DEBUG
            std::cout << "Pairing " << vertex_a << "<->" << vertex_b << '\n';
            #endif

            if (vertex_a < vertex_b && vertex_b < vertexCount) {
                indices.push_back(vertex_a);
                indices.push_back(vertex_b);

                #ifdef DEBUG
                std::cout << " * It's a match!\n";
                #endif
            }
        }
    }

    object->setVertexData(vertices.data(), vertices.size(), indices.data(), indices.size());
}

#else
void generateSolutionObject(Object* object, const std::vector<float> vertices) {};
void generateSolutionVector(Object* object, const glm::vec3 solutionVector) {};
void generateSolutionWireframe(Object* object, const std::vector<float> vertices, const std::vector<std::vector<int>> adjacency) {};
#endif

const glm::vec3 worldUp({0, 0, 1});

// class Display {
// private:

void Display::createObjects() {
    Display::planeObject.reset(new Object());

    VertexAttributePosition planeVertexData[] = {
        {-1.0, -1.0, 0.0},
        {-1.0,  1.0, 0.0},
        { 1.0, -1.0, 0.0},
        { 1.0,  1.0, 0.0}
    };

    int planeIndices[] = {
        0, 1, 3,
        0, 3, 2
    };

    Display::planeObject->setVertexData(planeVertexData, 4, planeIndices, 6);

    Display::vectorDisplay.reset(new Object());

    VertexAttributePosition vectorVertexData[13] = {
        /// /// /// /// ///
        { -1.0, -1.0,  0.0}, // 0
        { -1.0,  1.0,  0.0}, // 1
        {  1.0, -1.0,  0.0}, // 2
        {  1.0,  1.0,  0.0}, // 3
        { -1.0, -1.0,  1.0}, // 4
        { -1.0,  1.0,  1.0}, // 5
        {  1.0, -1.0,  1.0}, // 6
        {  1.0,  1.0,  1.0}, // 7
        /// /// /// /// ///
        {  0.0,  1.0, -1.0}, // 8
        {  1.0,  0.0, -1.0}, // 9
        {  0.0, -1.0, -1.0}, // 10
        { -1.0,  0.0, -1.0}, // 11
        {  0.0,  0.0,  1.0}  // 12
        /// /// /// /// ///
    };

    int vectorIndices[54] = {
        /// /// /// /// ///
         0,  6,  4,
         3,  6,  2,
         7,  4,  6,
         1,  7,  3,
         1,  2,  0,
         5,  0,  4,
         1,  0,  5,
         3,  2,  1,
         7,  6,  3,
         5,  4,  7,
         2,  6,  0,
         5,  7,  1,
        /// /// /// /// ///
         8,  9, 10,
         8, 10, 11,
         8,  9, 12,
         9, 10, 12,
        10, 11, 12,
        11,  8, 12
        /// /// /// /// ///
    };

    Display::vectorDisplay->setVertexData(vectorVertexData, 13, vectorIndices, 54);
}

void Display::createShaders() {
    #ifdef USE_BAKED_SHADERS
    Display::planeShader.reset(Shader::fromSource(shaders::vertex.plane_vert, shaders::fragment.plane_frag));
    Display::solutionShader.reset(Shader::fromSource(shaders::vertex.default_vert, shaders::fragment.default_frag));
    #else
    Display::planeShader.reset(new Shader("assets/plane.vert", "assets/plane.frag"));
    Display::solutionShader.reset(new Shader("assets/default.vert", "assets/default.frag"));
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

void Display::recalculateOptimalPlan() {
    optimalPlanTransform = glm::mat4(1);
    glm::vec3 optimalVectorNormal = glm::normalize(this->solution.optimalVector);

    if (optimalVectorNormal == worldUp) {
        optimalPlanTransform[1] = { 0, -optimalVectorNormal.z, 0, 0 };
    } else {
        glm::vec3 right = glm::normalize(glm::cross(optimalVectorNormal, worldUp));
        glm::vec3 up = glm::normalize(glm::cross(right, optimalVectorNormal));

        optimalPlanTransform[0] = { right, 0 };
        optimalPlanTransform[1] = { up, 0 };
        optimalPlanTransform[2] = { optimalVectorNormal, 0 };
    }
}

// TODO: Implement with instanced rendering
void Display::rebindAttributes() {};
void Display::onSolutionSolved() {
    if (!solutionWireframe) solutionWireframe.reset(new Object());
    if (!solutionObject) solutionObject.reset(new Object());
    generateSolutionWireframe(solutionWireframe.get(), solution.polyhedraVertices, solution.adjacency);
    generateSolutionObject(solutionObject.get(), solution.polyhedraVertices);
    recalculateOptimalPlan();
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
    if (!Display::planeObject || !Display::vectorDisplay) createObjects();
    if (!Display::planeShader || !Display::solutionShader) createShaders();
    this->showPlanesAtAll = true;
    this->visibleEquations = std::vector<bool>();
};

void Display::setScale(double scale) {
    this->globalScale = scale;
    this->globalScaleTransform = glm::scale(glm::mat4(1), glm::vec3(1 / this->globalScale));
}

void Display::render(Camera* camera) {
    if (planeTransforms.empty()) return;
    // glBindVertexArray(planeObject->objectData);
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
    this->planeShader->setUniform("globalScale", globalScaleTransform);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int planeIndex = 0; planeIndex < planeTransforms.size(); planeIndex++) {
        if (!visibleEquations[planeIndex]) continue;
        this->planeShader->setUniform("planeTransform", planeTransforms[planeIndex]);
        this->planeShader->setUniform("stripeScale", stripeFrequency);
        this->planeShader->setUniform("stripeWidth", stripeWidth);
        this->planeShader->setUniform("positiveColor", constraintPositiveColors[planeIndex % constraintPositiveColors.size()]);
        this->planeShader->setUniform("negativeColor", glm::vec3(1) - constraintPositiveColors[planeIndex % constraintPositiveColors.size()]);
        planeObject->bindForDraw();
    }
    // glDisable(GL_BLEND);
    }

    if (this->solution.isSolved && this->solutionObject) {
    this->solutionShader->activate();
    this->solutionShader->setTransform(camera->getProjection(), camera->getView());
    this->solutionShader->setUniform("transform", globalScaleTransform);
    if (this->showSolutionVolume) {
        this->solutionShader->setUniform("vertexColor", solutionColor);
        this->solutionObject->bindForDraw();
    }
    if (this->showSolutionWireframe) {
        this->solutionShader->setUniform("vertexColor", solutionWireframeColor);
        glLineWidth(this->wireThickness);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0, -11.0);

        this->solutionWireframe->bindForDraw(GL_LINES);

        glDisable(GL_POLYGON_OFFSET_LINE);
        glLineWidth(1.0);
    }
    if (this->showSolutionVector) {
        this->solutionShader->setUniform("vertexColor", this->solutionVectorColor);

        glm::vec3 vectorBaseScale = {
            this->vectorWidth,
            this->vectorWidth,
            glm::length(this->solution.optimalVector) - this->vectorWidth * this->arrowScale
        };
        glm::mat4 vectorBaseTransform = glm::scale(
            this->optimalPlanTransform,
            glm::vec3(vectorBaseScale)
        );

        this->solutionShader->setUniform("transform", vectorBaseTransform * globalScaleTransform);
        vectorDisplay->bindForDrawSlice(0, 36);

        glm::vec3 vectorArrowScale = glm::vec3(this->vectorWidth) * this->arrowScale;
        glm::mat4 vectorArrowTransform;
        vectorArrowTransform = vectorArrowTransform * globalScaleTransform; // even before anything, we apply global scale
        vectorArrowTransform = glm::translate(vectorArrowTransform, this->solution.optimalVector); // We move first
        vectorArrowTransform = vectorArrowTransform * this->optimalPlanTransform; // Then we reorient it to look in the direction
        vectorArrowTransform = glm::scale(vectorArrowTransform, vectorArrowScale); // And only then we scale
        this->solutionShader->setUniform("transform", vectorArrowTransform);
        vectorDisplay->bindForDrawSlice(36, 18);
    }
    }
};

Display::~Display() {
    Display::solutionShader.reset();
    Display::solutionObject.reset();
    Display::solutionWireframe.reset();

    Display::vectorDisplay.reset();

    Display::planeObject.reset();
    Display::planeShader.reset();
};
// };

// class WorldGridDisplay {
//     private:

void WorldGridDisplay::createObjects() {
    WorldGridDisplay::gridObject.reset(new Object());

    VertexAttributePosition gridVertexData[] = {
        {-1.0, -1.0, 0.0},
        {-1.0,  1.0, 0.0},
        { 1.0, -1.0, 0.0},
        { 1.0,  1.0, 0.0}
    };

    int gridIndices[] = {
        0, 1, 3,
        0, 3, 2
    };

    WorldGridDisplay::gridObject->setVertexData(gridVertexData, 4, gridIndices, 6);

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

    WorldGridDisplay::axisObject->setVertexData(axisVertexData, 4, axisIndices, 6);
}

void WorldGridDisplay::createShaders() {
    #ifdef USE_BAKED_SHADERS
    WorldGridDisplay::gridShader.reset(Shader::fromSource(shaders::vertex.grid_vert, shaders::fragment.grid_frag));
    WorldGridDisplay::axisShader.reset(Shader::fromSource(shaders::vertex.axis_vert, shaders::fragment.axis_frag));
    #else
    WorldGridDisplay::gridShader.reset(new Shader("assets/grid.vert", "assets/grid.frag"));
    WorldGridDisplay::axisShader.reset(new Shader("assets/axis.vert", "assets/axis.frag"));
    #endif
}

// class WorldGridDisplay
//     public:

WorldGridDisplay::WorldGridDisplay() {
    if (!WorldGridDisplay::gridObject || !WorldGridDisplay::axisObject) WorldGridDisplay::createObjects();
    if (!WorldGridDisplay::gridShader || !WorldGridDisplay::axisShader) WorldGridDisplay::createShaders();
}

void WorldGridDisplay::zoomGrid(float amount) {
    gridScale += amount * gridScale;
    if (gridScale > 10.0) {
        gridScale = 1.0;
        this->scaleExponent += 1;
    }
    if (gridScale <  0.1) {
        gridScale = 1.0;
        this->scaleExponent -= 1;
    }
}

double WorldGridDisplay::getComputedScale() {
    return pow(10, this->scaleExponent) * this->gridScale;
}

void WorldGridDisplay::render(glm::mat4 view, glm::mat4 projection) {
    if (gridEnabled) {
        gridShader->activate();
        gridShader->setTransform(projection, view);
        gridShader->setUniform("gridScale", gridScale);
        gridShader->setUniform("strokeWidth", gridWidth);
        gridShader->setUniform("gridOffset[0]", glm::vec2({ 1.0,  1.0}));
        gridShader->setUniform("gridOffset[1]", glm::vec2({ 1.0, -1.0}));
        gridShader->setUniform("gridOffset[2]", glm::vec2({-1.0,  1.0}));
        gridShader->setUniform("gridOffset[3]", glm::vec2({-1.0, -1.0}));

        gridObject->bindForDrawInstanced(4);

        if (gridScale >= 1.0) {
            gridShader->setUniform("gridScale", gridScale / 10);
            gridShader->setUniform("strokeWidth", 0.05 - gridWidth / gridScale);
            gridObject->bindForDrawInstanced(4);
        } else {
            gridShader->setUniform("gridScale", gridScale * 10);
            gridShader->setUniform("strokeWidth", 0.05 - gridWidth * gridScale);
            gridObject->bindForDrawInstanced(4);
        }
    }
    if (axisEnabled) {
        axisShader->activate();
        axisShader->setTransform(projection, view);
        axisObject->bindForDraw(GL_LINES);
    }
}

WorldGridDisplay::~WorldGridDisplay() {
    WorldGridDisplay::gridObject.reset();
    WorldGridDisplay::axisObject.reset();
    WorldGridDisplay::gridShader.reset();
    WorldGridDisplay::axisShader.reset();
}
// };
