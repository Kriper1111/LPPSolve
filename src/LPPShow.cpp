#include <vector>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "assets.h"
#include "camera.h"
#include "solver.h"


const float movementSpeed = 2.5f;
ImVec2 windowPosition = { 600, 0 };
ImVec2 windowSize = { 384, 720 };// { 360, 720 };
ImVec4 worldColor = { 0.364, 0.674, 0.764, 1.0 };

struct MouseState {
    bool leftMouseButton;
    bool rightMouseButton;
    bool middleMouseButton;

    double positionX;
    double positionY;
    double deltaX;
    double deltaY;
} mouseState;

namespace SceneData {
    bool canMoveCamera = true;
    bool allowEditCamera = false;
    bool showDebugOverlay = false;
    LinearProgrammingProblemDisplay* limits;
    WorldGridDisplay* worldOrigin;
    Camera *sceneCamera;
}

void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

int logCriticalError(const char* description) {
    std::cerr << "Critical: " << description << std::endl;
    // if(SceneData::hasOpenGlContext)
    glfwTerminate();
    return -1;
}

void moveCamera(Camera* camera, GLFWwindow* inputWindow, float timeStep) {
    float speedMod = (glfwGetKey(inputWindow, GLFW_KEY_LEFT_SHIFT) | glfwGetKey(inputWindow, GLFW_KEY_RIGHT_SHIFT)) ? 4.0f : 1.0f;
    speedMod *= timeStep * 1000.0f;

    float horizontal = (glfwGetKey(inputWindow, GLFW_KEY_D) - glfwGetKey(inputWindow, GLFW_KEY_A))
                     + (glfwGetKey(inputWindow, GLFW_KEY_LEFT) - glfwGetKey(inputWindow, GLFW_KEY_RIGHT));
    float vertical = (glfwGetKey(inputWindow, GLFW_KEY_W) - glfwGetKey(inputWindow, GLFW_KEY_S))
                     + (glfwGetKey(inputWindow, GLFW_KEY_UP) - glfwGetKey(inputWindow, GLFW_KEY_DOWN));


    if (horizontal || vertical) {
        // BUG: seems to be inverted on Windows for some reason.
        camera->orbit(horizontal * movementSpeed * speedMod, -vertical * movementSpeed * speedMod);
        // camera->setPerspective();
    }
}

void updateProcessDraw(GLFWwindow* window, Camera* camera, float timeStep) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Planes", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowPos(windowPosition);
    ImGui::SetWindowSize(windowSize);

    if (ImGui::CollapsingHeader("Camera controls")) {
        if (SceneData::allowEditCamera) {
            auto cameraLocation = camera->getCameraLocation();
            auto cameraRotation = camera->getCameraRotation();
            if (ImGui::InputFloat3("CameraPos:", &cameraLocation[0]))
                camera->teleportTo(cameraLocation.x, cameraLocation.y, cameraLocation.z);
            if (ImGui::InputFloat3("CameraLook:", &cameraRotation[0]))
                camera->rotate(cameraRotation.x, cameraRotation.y, cameraRotation.z);
        } else {
            ImGui::Text("CameraPos: %s", glx_toString(camera->getCameraLocation()).c_str());
            ImGui::Text("CameraLook: %s", glx_toString(camera->getCameraRotation()).c_str());
        }
        ImGui::Text("CameraDirect: %s", glx_toString(camera->getCameraDirection()).c_str());
        ImGui::Text("Movement speed: %.3f", movementSpeed);
        ImGui::SliderFloat("Look insensitivity", &camera->lookInSensitivity, 1.0f, 50.0f);
        ImGui::SliderFloat("Look depth", &camera->lookDepth, 0.0f, 100.0f);
        ImGui::Checkbox("Allow editing camera values", &SceneData::allowEditCamera);
        // bool isOrtho = camera->useOrthography();
        // if (ImGui::Checkbox("Use orthography", &isOrtho)) {
        //     camera->useOrthography(isOrtho);
        // }
        
        if (ImGui::Button("up X View")) {
            camera->teleportTo(camera->lookDepth, 0, 0);
            camera->rotate(0, 0, -90);
            // camera->setOrtography();
        }
        if (ImGui::Button("down Y View")) {
            camera->teleportTo(0, -camera->lookDepth, 0);
            camera->rotate(0, 0, 0);
            // camera->setOrtography();
        }
        if (ImGui::Button("down Z view")) {
            camera->teleportTo(0, 0, camera->lookDepth);
            camera->rotate(-89.5, 0, 0);
            // camera->setOrtography();
        }
    }

    if (ImGui::CollapsingHeader("World settings")) {
        #ifdef DEBUG
        ImGui::ColorPicker4("World color", &worldColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoAlpha);
        #endif
        ImGui::Checkbox("Show grid", &SceneData::worldOrigin->gridEnabled);
        ImGui::Checkbox("Show world axis", &SceneData::worldOrigin->axisEnabled);
        ImGui::SliderFloat("Grid scale", &SceneData::worldOrigin->zoomScale, 0.1f, 10.0f);
    }

    ImGui::Text("Total planes: %d", SceneData::limits->getEquationCount());
    if (ImGui::Button("Add plane") && SceneData::limits->getEquationCount() < 256) {
        SceneData::limits->addLimitPlane({0, 0, 1, 0});
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove plane") && SceneData::limits->getEquationCount() > 0) {
        SceneData::limits->removeLimitPlane();
    }
    ImGui::Separator();

    glm::vec4 targetFunctionEq = SceneData::limits->getTargetFunction();
    ImGui::InputFloat4("Target function", &targetFunctionEq[0]);
    if (ImGui::IsItemEdited()) {
        SceneData::limits->setTargetFunction(targetFunctionEq);
    }
    ImGui::Separator();

    // TODO: Make into a scrollbox
    for (int planeIndex = 0; planeIndex < SceneData::limits->getEquationCount(); planeIndex++) {
        glm::vec4 planeEquationOrigin = SceneData::limits->getLimitPlane(planeIndex);
        ImGui::Text("Plane: ");
        ImGui::PushID(planeIndex);
        if (ImGui::InputFloat4("Equation", &planeEquationOrigin[0])) {
            SceneData::limits->editLimitPlane(planeIndex, planeEquationOrigin);
        }
        ImGui::PopID();
    }

    if (ImGui::Button("Solve")) {
        try {
            SceneData::limits->solve();
        } catch (std::runtime_error &dd_error) {
            ImGui::TextColored({0.918, 0.025, 0.163, 1.0}, "Failed to solve the equation: %s", dd_error.what());
        }
    }
    if (SceneData::limits->isSolved()) {
        ImGui::Text("Optimal value: %.4f", SceneData::limits->getOptimalValue());
        ImGui::Text("Optimal plan: %.3fx1 %.3fx2 %.3fx3 %.3f", SceneData::limits->getOptimalVertex());
    }

    #ifdef DEBUG
    ImGui::Checkbox("Show debug overlay (imgui)", &SceneData::showDebugOverlay);
    ImGui::Checkbox("Toggle freecam", &SceneData::canMoveCamera);

    if (SceneData::showDebugOverlay) ImGui::ShowMetricsWindow(&SceneData::showDebugOverlay);
    #endif

    ImGui::End();

    if (SceneData::canMoveCamera) moveCamera(camera, window, timeStep);
    SceneData::limits->renderLimitPlanes(camera->getView(), camera->getProjection());
    SceneData::worldOrigin->render(camera->getView(), camera->getProjection());

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void glfwWindowResizeCallback(GLFWwindow *window, int width, int height) {
    windowSize.x = width * 0.4;
    windowSize.y = height;

    windowPosition.x = width - windowSize.x;
    windowPosition.y = 0; // height - windowSize.y;
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
    // XXX: Make GLFW init thing into an object?
    // That way we could use auto-destructors without worrying about 
    // deleting stray Objects after glfw has been deinitialized.
    // We could probably make it into a thin wrapper
    // Maybe even overrideable, with a .init where we could set up ImGui, for example.
    ////////////
    //// INIT GLFW
    ////////////

    if(glfwInit() == GLFW_FALSE) { std::cerr << "Failed to init GLFW" << std::endl; return -1; }

    glfwSetErrorCallback(glfwErrorCallback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // OpenGL 3.3 core

    int windowWidth = 960;
    int windowHeight = 720;

    GLFWwindow* mainWindow = glfwCreateWindow(windowWidth, windowHeight, "Linear Programming Problem: Show", NULL, NULL);
    if (mainWindow == nullptr) return logCriticalError("Failed to create a window");
    // ...
    glfwSetWindowSizeCallback(mainWindow, glfwWindowResizeCallback);
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
    // ImGuiIO& iio = ImGui::GetIO(); (void) iio;
    // No need for custom fonts, for now

    ////////////
    //// SCENE SETUP
    ////////////
    Camera* camera = new Camera(windowWidth, windowHeight);
    camera->teleportTo(7.35889, 6.92579, 4.95831);
    camera->rotate(-26.4407, 0.0, -133.3081);
    // 7.35889 m, -6.92579 m, 4.95831 m // X, -Y, Z
    // 63.5593°, 0°, 46.6919° // pitch - 90, 0, yaw - 180
    camera->lookDepth = glm::length(camera->getCameraLocation());
    SceneData::sceneCamera = camera;

    try {
        SceneData::limits = new LinearProgrammingProblemDisplay();
        SceneData::worldOrigin = new WorldGridDisplay();
    } catch (std::exception &ioerr) {
        delete SceneData::limits;
        delete SceneData::worldOrigin;
        // Might get to segfault
        return logCriticalError("Failed to compile required shaders");
    }

    // ???
    glfwWindowResizeCallback(mainWindow, windowWidth, windowHeight);

    float lastFrame, deltaTime = 0;

    while (!glfwWindowShouldClose(mainWindow)) {
        float time = glfwGetTime();
        deltaTime = time - lastFrame;
        lastFrame = time;

        glClearColor(worldColor.x, worldColor.y, worldColor.z, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // XXX: wonky with the built-in callback, maybe because data races?
        //      Shouldn't happen though, I imagine the callbacks are executed
        //      during glfwPollEvents();
        //      well it says that right in the description that they don't
        //      necessary get called when glfwPollEvents(); is executed
        glfwMouseCallback(mainWindow);
        updateProcessDraw(mainWindow, camera, deltaTime);

        // FIXME: Needs VSync on Windows
        glfwSwapBuffers(mainWindow);
        glfwPollEvents();
    }

    // We have to delete them before we deinit glfw and exit the program scope (and consequently opengl)
    // As otherwise we attempt to asl now unloaded GL context to deallocate the object and shader buffers
    delete SceneData::limits;
    delete SceneData::worldOrigin;
    glfwTerminate();
}