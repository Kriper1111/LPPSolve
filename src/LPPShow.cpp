#include <vector>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>

// #include <cddlib/cdd.h>

#include "assets.h"
#include "camera.h"
#include "solver.h"


const glm::vec3 zeroVector({0, 0, 0});
const glm::vec3 upVector({0, 0, 1});

struct MouseState {
    bool leftMouseButton;
    bool rightMouseButton;
    bool middleMouseButton;

    double positionX;
    double positionY;
    double deltaX;
    double deltaY;
} mouseState;

// class WorldOrigin {
//     private:
//     static Object* worldObject;

//     void buildWorldOrigin() {
//         WorldOrigin::worldObject = new Object();
//         float worldOriginData[] = {
//             0.0, 0.0, 0.0, 0.0, 0.0,
//             0.0, 0.0, 1.0, 0.0, 0.0,
//             0.0, 1.0, 0.0, 0.0, 0.0,
//             1.0, 0.0, 0.0, 0.0, 0.0
//         };
//         int worldOriginIndices[] = {
//             0, 1,
//             0, 2,
//             0, 3
//         };

//         glGenVertexArrays(1, &worldObject->objectData);
//         glGenBuffers(1, &worldObject->vertexData);
//         glGenBuffers(1, &worldObject->indices);

//         glBindVertexArray(worldObject->objectData);
//         glBindBuffer(GL_ARRAY_BUFFER, worldObject->vertexData);
//         glBufferData(GL_ARRAY_BUFFER, sizeof(worldOriginData), worldOriginData, GL_STATIC_DRAW);
//         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
//         glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
//         glEnableVertexAttribArray(0);
//         glEnableVertexAttribArray(1);

//         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, worldObject->indices);
//         glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(worldOriginIndices), worldOriginIndices, GL_STATIC_DRAW);

//         worldObject->vertexCount = 4;
//     }

//     public:
//     WorldOrigin() {
//         if (WorldOrigin::worldObject == nullptr) buildWorldOrigin();
//     }

//     void render() {
//         glUseProgram(0);
//         glBindVertexArray(WorldOrigin::worldObject->objectData);
//         glDrawElements(GL_LINE, WorldOrigin::worldObject->vertexCount, GL_UNSIGNED_INT, 0);
//     }

// };

// Object* WorldOrigin::worldObject = nullptr;

namespace runtime {
    bool canMoveCamera;
    bool showDebugOverlay;
    LinearProgrammingProblemDisplay* limits;
    Camera *sceneCamera;
    Object *cube;
    Shader *cubeShader;
}

void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

int logCriticalError(const char* description) {
    std::cerr << "Critical: " << description << std::endl;
    return -1;
}

void moveCamera(Camera* camera, GLFWwindow* inputWindow, float timeStep) {
    float speedMod = (glfwGetKey(inputWindow, GLFW_KEY_LEFT_SHIFT) | glfwGetKey(inputWindow, GLFW_KEY_RIGHT_SHIFT)) ? 4.0f : 1.0f;

    float horizontal = (glfwGetKey(inputWindow, GLFW_KEY_D) - glfwGetKey(inputWindow, GLFW_KEY_A))
                     + (glfwGetKey(inputWindow, GLFW_KEY_LEFT) - glfwGetKey(inputWindow, GLFW_KEY_RIGHT));
    float vertical = (glfwGetKey(inputWindow, GLFW_KEY_W) - glfwGetKey(inputWindow, GLFW_KEY_S))
                     + (glfwGetKey(inputWindow, GLFW_KEY_UP) - glfwGetKey(inputWindow, GLFW_KEY_DOWN));

    if (horizontal || vertical) {
        camera->orbit(horizontal * 5 * speedMod, vertical * 5 * speedMod);
    }
}

void updateProcessDraw(GLFWwindow* window, Camera* camera, float timeStep) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Planes");

    if (ImGui::CollapsingHeader("Camera controls")) {
        ImGui::Text("CameraPos: %s", glx_toString(camera->getCameraLocation()).c_str());
        ImGui::Text("CameraLook: %s", glx_toString(camera->getCameraRotation()).c_str());
        ImGui::Text("CameraDirect: %s", glx_toString(camera->getCameraDirection()).c_str());
        ImGui::SliderFloat("Look insensitivity", &camera->lookInSensitivity, 1.0f, 50.0f);
        ImGui::SliderFloat("Look depth", &camera->lookDepth, 0.0f, 100.0f);
    }

    // bool dirty = false;
    // if (ImGui::CollapsingHeader("Orientation master")) {
    //     ImGui::Text("Target Axis");
    //     if (ImGui::BeginTable("tar", 6, ImGuiTableFlags_NoSavedSettings)) {
    //         int targetAxis = LimitPlane::getTargetAxis();
    //         for (int axis = 0; axis < 6; axis++) {
    //             ImGui::TableNextColumn();
    //             if (ImGui::Selectable(LimitPlane::targetAxies[axis].label, targetAxis == axis)) {
    //                 LimitPlane::setTargetAxis(axis);
    //                 dirty = true;
    //             }
    //         }
    //         ImGui::EndTable();
    //     }
    //     ImGui::Text("Track Axis");
    //     if (ImGui::BeginTable("tra", 3, ImGuiTableFlags_NoSavedSettings)) {
    //         int trackAxis = LimitPlane::getTrackAxis();
    //         for (int axis = 0; axis < 3; axis++) {
    //             ImGui::TableNextColumn();
    //             if (ImGui::Selectable(LimitPlane::trackAxies[axis].label, trackAxis == axis)) {
    //                 LimitPlane::setTrackAxis(axis);
    //                 dirty = true;
    //             }
    //         }
    //         ImGui::EndTable();
    //     }
    // }

    ImGui::Text("Total planes: %d", runtime::limits->getEquationCount());
    if (ImGui::Button("Add plane") && runtime::limits->getEquationCount() < 256) {
        runtime::limits->addLimitPlane({0, 0, 1, 0});
        // runtime::limits->push_back(new LimitPlane(0, 0, 1, 0));
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove plane") && runtime::limits->getEquationCount() > 0) {
        runtime::limits->removeLimitPlane();
        // runtime::limits->pop_back();
    }
    ImGui::Separator();

    for (int planeIndex = 0; planeIndex < runtime::limits->getEquationCount(); planeIndex++) {
        glm::vec4 planeEquationOrigin = runtime::limits->getLimitPlane(planeIndex);
        // glm::vec4 planeEquationNew = glm::vec4(planeEquationOrigin);
        ImGui::Text("Plane: ");
        ImGui::PushID(planeIndex);
        ImGui::InputFloat4("Equation", &planeEquationOrigin[0]);
        ImGui::PopID();
        if (ImGui::IsItemEdited()) {
            runtime::limits->editLimitPlane(planeIndex, planeEquationOrigin);
            // runtime::limits[planeIndex]->recalculateEquation(planeEquationOrigin);
        }
    }

    ImGui::Checkbox("Show debug overlay (imgui)", &runtime::showDebugOverlay);
    ImGui::Checkbox("Toggle freecam", &runtime::canMoveCamera);

    if (runtime::showDebugOverlay) ImGui::ShowMetricsWindow(&runtime::showDebugOverlay);

    ImGui::End();

    if (runtime::canMoveCamera) moveCamera(camera, window, timeStep);
    runtime::limits->renderLimitPlanes(camera->getView(), camera->getProjection());
    // for (LimitPlane* plane : runtime::limits) { plane->render(camera); }
    // runtime::worldOrigin->render();

    // double currentX; // = mouseState.positionX;
    // double currentY; // = mouseState.positionY;
    // glfwGetCursorPos(window, &currentX, &currentY);
    // mouseState.deltaX = previousX - mouseState.positionX;
    // mouseState.deltaY = previousY - mouseState.positionY;

    // mouseState.positionX = mouseState.positionX;
    // mouseState.positionY = mouseState.positionY;

    // mouseState.leftMouseButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    // mouseState.rightMouseButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    // mouseState.middleMouseButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);

    runtime::cubeShader->activate();
    runtime::cubeShader->setTransform(camera->getProjection(), camera->getView());
    glBindVertexArray(runtime::cube->objectData);
    glDrawElements(GL_TRIANGLES, runtime::cube->vertexCount, GL_UNSIGNED_INT, 0);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void glfwMouseCallback(GLFWwindow* window, double positionX, double positionY) {
    mouseState.deltaX = mouseState.positionX - positionX;
    mouseState.deltaY = mouseState.positionY - positionY;

    mouseState.positionX = positionX;
    mouseState.positionY = positionY;

    mouseState.leftMouseButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    mouseState.rightMouseButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    mouseState.middleMouseButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
}

void glfwMouseCallback(GLFWwindow* window) {
    double positionX, positionY;
    glfwGetCursorPos(window, &positionX, &positionY);
    glfwMouseCallback(window, positionX, positionY);
}

int main() {
    ////////////
    //// INIT GLFW
    ////////////

    if(glfwInit() == GLFW_FALSE) { std::cerr << "Failed to init GLFW" << std::endl; return -1; }

    glfwSetErrorCallback(glfwErrorCallback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // OpenGL 3.3 core

    int windowWidth = 640;
    int windowHeight = 480;

    GLFWwindow* mainWindow = glfwCreateWindow(windowWidth, windowHeight, "Linear Programming Problem: Show", NULL, NULL);
    if (mainWindow == nullptr) return logCriticalError("Failed to create a window");
    // ...
    // glfwSetInputMode(mainWindow, GLFW_STICKY_MOUSE_BUTTONS, GLFW_FALSE);
    // glfwSetCursorPosCallback(mainWindow, glfwMouseCallback);
    // ...
    glfwMakeContextCurrent(mainWindow);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return logCriticalError("Failed to get OpenGL context");

    // initApplication();

    ////////////
    //// INIT OPENGL
    ////////////
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);

    ////////////
    //// INIT IMGUI
    ////////////
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGuiIO& iio = ImGui::GetIO(); (void) iio; // what does that do?

    Camera* camera = new Camera(windowWidth, windowHeight);
    camera->teleportTo(7.35889, -6.92579, 4.95831);
    camera->rotate(-26.4407, 0.0, -46.6919);// 46.6919);
    camera->lookDepth = glm::length(camera->getCameraLocation());
    runtime::sceneCamera = camera;
    Object* cube = new Object();
    fromWavefront(cube, "assets/the world.obj");

    runtime::cubeShader = new Shader("assets/default.vert", "assets/default.frag");
    runtime::cube = cube;
    runtime::limits = new LinearProgrammingProblemDisplay();

    float lastFrame, deltaTime = 0;
    // runtime::worldOrigin = new WorldOrigin();

    while (!glfwWindowShouldClose(mainWindow)) {
        float time = glfwGetTime();
        deltaTime = time - lastFrame;
        lastFrame = time;

        glClearColor(0.176, 0.487, 0.801, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwMouseCallback(mainWindow);
        updateProcessDraw(mainWindow, camera, deltaTime);

        glfwSwapBuffers(mainWindow);
        glfwPollEvents();
    }

    delete runtime::limits;
    // LimitPlane::deallocate();
    // std::cout << "GLFW terminated" << std::endl;
    glfwTerminate();
}