#include <iostream>
#include <vector>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#define LOCALMAN_IMPL
#define DISPLAY_IMPL
#include "localman.h"
#include "assets.h"
#include "camera.h"
#include "solver.h"
#include "config.h"

// *honestly* i should define a function doing the same as '_' from moFileReader (mfr for short)
// instead of making it a macro that way it's.. i guess more control
#define l10n(str) moFileLib::_(str)
#define l10nc(str) moFileLib::_(str).c_str()
#define l10nm(str) str
#define l10nd(str) moFileLib::_(str).c_str()
// last one is for deferred localization

const float movementSpeed = 2.5f;
ImVec2 imguiWindowPosition = { 980, 20 };
ImVec2 imguiWindowSize = { 320, 720 };
ImVec4 worldColor = { 0.364, 0.674, 0.764, 1.0 };
const char* const minmax[2] = { "min", "max" };
const char* const equ[3] = { "<=", ">=", "=" };

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
    Display* lppshow;
    WorldGridDisplay* worldOrigin;
    Camera *sceneCamera;
}

namespace SettingsWindow {
    bool showSettingsWindow = false;
    const char* editColors = l10nm("Colors");
    const char* editGeneral = l10nm("General");
    const char* editSliders = l10nm("Sliders");
    const char* editVisibility = l10nm("Visibility");

    const std::vector<const char*> options = {
        editGeneral,
        editColors,
        editSliders,
        editVisibility
    };
    const char* selectedOption = options[0];

    // readability
    bool selected(const char* option) { return selectedOption == option; }
}

void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

int logCriticalError(const char* description) {
    std::cerr << "Critical: " << description << std::endl;
    glfwTerminate();
    return -1;
}

void moveCamera(Camera* camera, GLFWwindow* inputWindow, float timeStep) {
    float speedMod = (glfwGetKey(inputWindow, GLFW_KEY_LEFT_SHIFT) |
                      glfwGetKey(inputWindow, GLFW_KEY_RIGHT_SHIFT)) ? 4.0f : 1.0f;
    speedMod *= timeStep * 1000.0f;

    float orbitHorizontal = glfwGetKey(inputWindow, GLFW_KEY_LEFT) - glfwGetKey(inputWindow, GLFW_KEY_RIGHT);
    float orbitVertical = glfwGetKey(inputWindow, GLFW_KEY_UP) - glfwGetKey(inputWindow, GLFW_KEY_DOWN);

    float moveForwards = glfwGetKey(inputWindow, GLFW_KEY_W) - glfwGetKey(inputWindow, GLFW_KEY_S);
    float moveLateral = glfwGetKey(inputWindow, GLFW_KEY_D) - glfwGetKey(inputWindow, GLFW_KEY_A);

    float zoom = (glfwGetKey(inputWindow, GLFW_KEY_R) - glfwGetKey(inputWindow, GLFW_KEY_F));

    float snapToTop = glfwGetKey(inputWindow, GLFW_KEY_KP_7);
    float snapToLeft = glfwGetKey(inputWindow, GLFW_KEY_KP_1);
    float snapToRight = glfwGetKey(inputWindow, GLFW_KEY_KP_3);

    if (snapToRight) {
        camera->teleportTo(camera->lookDepth, 0, 0);
        camera->rotate(0, 0, -90);
        camera->setOrtography();
    } else if (snapToLeft) {
        camera->teleportTo(0, -camera->lookDepth, 0);
        camera->rotate(0, 0, 0);
        camera->setOrtography();
    } else if (snapToTop) {
        camera->teleportTo(0, 0, camera->lookDepth);
        camera->rotate(-89.9, 0, 0);
        camera->setOrtography();
    }

    if (snapToLeft || snapToRight || snapToTop) {
        camera->setOrtography();
    } else if (orbitHorizontal || orbitVertical) {
        camera->orbit(orbitHorizontal * movementSpeed * speedMod, -orbitVertical * movementSpeed * speedMod);
        camera->setPerspective();
    }
    if (moveLateral || moveForwards) {
        camera->walk(
            moveForwards * speedMod * 0.01,
            moveLateral * speedMod * 0.01,
            0,
            { 1, 1, 0 }
        );
    }
    if (mouseState.leftMouseButton) {
        camera->orbit(-mouseState.deltaX * timeStep * 1000.0f, mouseState.deltaY * timeStep * 1000.0f);
        camera->setPerspective();
    }
    if (zoom) {
        SceneData::worldOrigin->zoomGrid(zoom * 0.005 * speedMod);
        SceneData::lppshow->setScale(SceneData::worldOrigin->getComputedScale());
    }
}

bool deserialize() { return true; };
bool serialize() { return true; };

void show_preferences_window(bool* isOpen) {
    if(!ImGui::Begin(l10n("Preferences").append("###preferences").c_str(), isOpen)) {
        ImGui::End();
        return;
    }

    ImGui::BeginChild("options", ImVec2(ImGui::GetContentRegionAvail().x * 0.25, ImGui::GetContentRegionAvail().y), true);
    // ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.25);
    for (auto option : SettingsWindow::options) {
        if (ImGui::Selectable(l10nd(option), SettingsWindow::selectedOption == option)) {
            SettingsWindow::selectedOption = option;
        }
        if (SettingsWindow::selectedOption == option) {
            ImGui::SetItemDefaultFocus();
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("settingsPane");
    if (SettingsWindow::selected(SettingsWindow::editGeneral)) {
        static int currentStyle = 0;
        ImGui::Text(l10nc("Color Theme: ")); ImGui::SameLine();
        if (ImGui::Combo("###color", &currentStyle, "ImGui Dark\0ImGui Light\0ImGui Classic\0")) {
            switch (currentStyle) {
                case 0: ImGui::StyleColorsDark(); break;
                case 1: ImGui::StyleColorsLight(); break;
                case 2: ImGui::StyleColorsClassic(); break;
            }
        }
        ImGui::Text(l10nc("Language: ")); ImGui::SameLine();
        if (ImGui::BeginCombo("###lang", LocalMan::currentLocale.c_str())) {
            for (const auto &locale : LocalMan::localesMap) {
                if (ImGui::Selectable(locale.first.c_str(), locale.first == LocalMan::currentLocale)) {
                    LocalMan::changeLocale(locale.first);
                }
            }
            ImGui::EndCombo();
        }
    }
    if (SettingsWindow::selected(SettingsWindow::editColors)) {
        ImGuiColorEditFlags shaderPickerFlags = ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions;
        #ifdef DEBUG
        shaderPickerFlags = shaderPickerFlags & (~ImGuiColorEditFlags_NoOptions);
        ImGui::ColorEdit3("World color", &worldColor.x, shaderPickerFlags);
        ImGui::Separator();
        #endif
        ImGui::ColorEdit3(l10nc("Feasible range"), &SceneData::lppshow->solutionColor.x, shaderPickerFlags);
        ImGui::ColorEdit3(l10nc("Feasible range edges"), &SceneData::lppshow->solutionWireframeColor.x, shaderPickerFlags);
        ImGui::ColorEdit3(l10nc("Solution vector"), &SceneData::lppshow->solutionVectorColor.x, shaderPickerFlags);
        for (int colorIndex = 0; colorIndex < SceneData::lppshow->constraintPositiveColors.size(); colorIndex++) {
            ImGui::PushID(colorIndex);
            ImGui::ColorEdit3(l10nc("Plane color"), &SceneData::lppshow->constraintPositiveColors[colorIndex].x, shaderPickerFlags);
            ImGui::PopID();
        }
    }
    if (SettingsWindow::selected(SettingsWindow::editSliders)) {
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.4);
        #ifdef DEBUG
        ImGui::Text("Computed grid scale %f", SceneData::worldOrigin->getComputedScale());
        ImGui::InputFloat("Grid width", &SceneData::worldOrigin->gridWidth, 0.01f, 0.015f);
        ImGui::SliderFloat("Vector width", &SceneData::lppshow->vectorWidth, 0.01f, 1.0f);
        ImGui::SliderFloat("Arrow scale", &SceneData::lppshow->arrowScale, 1.0f, 10.0f);
        ImGui::Separator();
        #endif
        ImGui::SliderFloat(l10nc("Plane stripe width"), &SceneData::lppshow->stripeWidth, 0.0f, 1.0f);
        ImGui::SliderFloat(l10nc("Plane stripe frequency"), &SceneData::lppshow->stripeFrequency, 1.0f, 100.0f);
        ImGui::SliderFloat(l10nc("Feasible range edge thickness"), &SceneData::lppshow->wireThickness, 1.0f, 5.0f);
        ImGui::PopItemWidth();
    }
    if (SettingsWindow::selected(SettingsWindow::editVisibility)) {
        ImGui::Checkbox(l10nc("Show grid"), &SceneData::worldOrigin->gridEnabled);
        ImGui::Checkbox(l10nc("Show world axis"), &SceneData::worldOrigin->axisEnabled);
        ImGui::Checkbox(l10nc("Show planes"), &SceneData::lppshow->showPlanesAtAll);
        ImGui::Checkbox(l10nc("Show feasible range"), &SceneData::lppshow->showSolutionVolume);
        ImGui::Checkbox(l10nc("Show feasible range edges"), &SceneData::lppshow->showSolutionWireframe);
        ImGui::Checkbox(l10nc("Show solution vector"), &SceneData::lppshow->showSolutionVector);
    }
    ImGui::EndChild();

    ImGui::End();
}

void updateProcessDraw(GLFWwindow* window, Camera* camera, float timeStep) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

    // TODO: Make collapsing too, maybe?
    ImGui::Begin(l10n("Planes").append("###planes-win").c_str(), nullptr, windowFlags);
    ImGui::SetWindowPos(imguiWindowPosition);
    ImGui::SetWindowSize(imguiWindowSize);

    bool shouldExit = false;
    bool setExample = false;

    if (ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            // ImGui::MenuItem("Open..", "o", &deserialize);
            // ImGui::MenuItem("Save..", "s", &serialize);
            ImGui::MenuItem("Load example..", nullptr, &setExample);
            ImGui::Separator();
            ImGui::MenuItem("Preferences", "p", &SettingsWindow::showSettingsWindow);
            ImGui::Separator();
            ImGui::MenuItem("Exit", "e", &shouldExit);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (shouldExit) {
        ImGui::End();
        glfwSetWindowShouldClose(window, 1);
        return;
    }
    if (setExample) {
        SceneData::lppshow->reset();
        SceneData::lppshow->addLimitPlane({0, 0, 1, 0}, EquationType::GREATER_EQUAL_THAN); SceneData::lppshow->visibleEquations[0] = false;
        SceneData::lppshow->addLimitPlane({0, 1, 0, 0}, EquationType::GREATER_EQUAL_THAN); SceneData::lppshow->visibleEquations[1] = false;
        SceneData::lppshow->addLimitPlane({1, 0, 0, 0}, EquationType::GREATER_EQUAL_THAN); SceneData::lppshow->visibleEquations[2] = false;
        SceneData::lppshow->addLimitPlane({1, 0, 0, 1});
        SceneData::lppshow->addLimitPlane({0, 1, 0, 1});
        SceneData::lppshow->addLimitPlane({0, 0, 1, 1});
        SceneData::lppshow->objectiveFunction = { 2, 3, 0, 0 };
        setExample = false;
    }
    if (SettingsWindow::showSettingsWindow) show_preferences_window(&SettingsWindow::showSettingsWindow);

    if (ImGui::CollapsingHeader(l10n("Camera controls").append("###camera-opt").c_str())) {
        #ifdef DEBUG
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
        #endif

        // Ortho
        float orthographicScale = camera->getOrthographicScale();
        if(ImGui::SliderFloat("Orthographic scale", &orthographicScale, 0.0f, 100.0f))
            camera->setOrthographicScale(orthographicScale);
        bool isOrtho = camera->useOrthography();
        if (ImGui::Checkbox("Use orthography", &isOrtho)) {
            camera->useOrthography(isOrtho);
        }

        if (ImGui::Button("up X View")) {
            camera->teleportTo(camera->lookDepth, 0, 0);
            camera->rotate(0, 0, -90);
            camera->setOrtography();
        }
        if (ImGui::Button("down Y View")) {
            camera->teleportTo(0, -camera->lookDepth, 0);
            camera->rotate(0, 0, 0);
            camera->setOrtography();
        }
        if (ImGui::Button("down Z view")) {
            camera->teleportTo(0, 0, camera->lookDepth);
            camera->rotate(-89.9, 0, 0);
            camera->setOrtography();
        }
    }

    ImGui::Text(l10nc("Objective function:"));
    ImGui::InputFloat4("##objective", &SceneData::lppshow->objectiveFunction.x);
    ImGui::SameLine(); ImGui::Text("->"); ImGui::SameLine();
    auto doMinimize = SceneData::lppshow->doMinimize;
    int currentItem = (int) !doMinimize;

    // XXX: This solution is much cleaner but lacks clear localization support
    ImGui::PushItemWidth(50.0f);
    if (ImGui::Combo("##min", &currentItem, minmax, 2))
        SceneData::lppshow -> doMinimize = !doMinimize;
    ImGui::PopItemWidth();
    ImGui::Separator();

    ImGui::Text("Total planes: %d", SceneData::lppshow->getEquationCount());
    if (ImGui::Button(l10nc("Add plane")) && SceneData::lppshow->getEquationCount() < 256) {
        SceneData::lppshow->addLimitPlane({0, 0, 1, 0});
    }
    ImGui::SameLine();
    if (ImGui::Button(l10nc("Remove plane")) && SceneData::lppshow->getEquationCount() > 0) {
        SceneData::lppshow->removeLimitPlane();
    }
    ImGui::Separator();

    // TODO: Make into a scrollbox
    float TEXT_BASE_WIDTH = ImGui::GetTextLineHeightWithSpacing();
    ImVec2 tableSize = ImVec2(0.0f, TEXT_BASE_WIDTH * 8);
    ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersH | ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoPadInnerX;
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(3, 0));
    if (ImGui::BeginTable("###plane-equations", 7, tableFlags, tableSize)) {
        ImGui::TableSetupScrollFreeze(1, 0);
        ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH);
        ImGui::TableSetupColumn("x₁");
        ImGui::TableSetupColumn("x₂");
        ImGui::TableSetupColumn("x₃");
        ImGui::TableSetupColumn("=", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 1.5);
        ImGui::TableSetupColumn("b");
        ImGui::TableSetupColumn("V", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH);
        ImGui::TableHeadersRow();
        ImGui::TableSetColumnIndex(1); ImGui::PushItemWidth(-FLT_MIN);
        ImGui::TableSetColumnIndex(2); ImGui::PushItemWidth(-FLT_MIN);
        ImGui::TableSetColumnIndex(3); ImGui::PushItemWidth(-FLT_MIN);
        ImGui::TableSetColumnIndex(4); ImGui::PushItemWidth(-FLT_MIN);
        ImGui::TableSetColumnIndex(5); ImGui::PushItemWidth(-FLT_MIN);
        ImGui::TableSetColumnIndex(6);


        // ImGui::
        for (int planeIndex = 0; planeIndex < SceneData::lppshow->getEquationCount(); planeIndex++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            auto planeEquationOrigin = SceneData::lppshow->getLimitPlane(planeIndex);
            ImGui::PushID(planeIndex);
            if (ImGui::Button("x")) SceneData::lppshow->editLimitPlane(planeIndex, {0, 0, 0, 0});
            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);

            bool coeffChanged = false;

            coeffChanged |= ImGui::InputFloat("##vecx1", &planeEquationOrigin.equationCoefficients[0]);
            ImGui::TableNextColumn();
            coeffChanged |= ImGui::InputFloat("##vecx2", &planeEquationOrigin.equationCoefficients[1]);
            ImGui::TableNextColumn();
            coeffChanged |= ImGui::InputFloat("##vecx3", &planeEquationOrigin.equationCoefficients[2]);
            ImGui::TableNextColumn();

            int currentType = planeEquationOrigin.type;
            bool typeChanged = false;

            // The only reason as to why I'm using is is the damn dropdown button.
            if (ImGui::BeginCombo("##type", equ[currentType], ImGuiComboFlags_NoArrowButton)) {
                for (int eqType = 0; eqType < 3; eqType++) {
                    bool itemSelected = currentType == eqType;
                    if (ImGui::Selectable(equ[eqType], itemSelected)) {
                        typeChanged = true;
                        planeEquationOrigin.type = static_cast<EquationType>(eqType);
                    }

                    if (itemSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::TableNextColumn();

            bool constChanged = ImGui::InputFloat("##const", &planeEquationOrigin.equationCoefficients[3]);
            ImGui::TableNextColumn();
            ImGui::PopStyleColor();

            // XXX: Do we want to use std::vector<bool> optimization or fall back to Plane objects?
            bool isVisible = SceneData::lppshow->visibleEquations[planeIndex];
            if(ImGui::Checkbox("##vis", &isVisible)) SceneData::lppshow->visibleEquations[planeIndex] = isVisible;
            ImGui::TableNextColumn();

            ImGui::PopID();

            if(coeffChanged || constChanged || typeChanged) {
                SceneData::lppshow->editLimitPlane(
                    planeIndex,
                    planeEquationOrigin.equationCoefficients,
                    planeEquationOrigin.type
                );
            }
        }
        ImGui::EndTable();
    }

    ImGui::PopStyleVar();
    if (ImGui::Button("+") && SceneData::lppshow->getEquationCount() < 256) {
        SceneData::lppshow->addLimitPlane({0, 0, 1, 0});
    }
    ImGui::SameLine(); ImGui::Text(l10nc("Add plane"));

    if (ImGui::Button(l10nc("Solve"))) {
        try {
            SceneData::lppshow->solve();
        } catch (std::runtime_error &dd_error) {
            std::cerr << "Faile to solve equation: " << dd_error.what() << std::endl;
        }
    }
    const auto *solution = SceneData::lppshow->getSolution();
    if (solution->isErrored) {
        ImGui::TextColored({0.918, 0.025, 0.163, 1.0}, l10nc("Failed to solve the equation: %s"), solution->errorString.c_str());
    } else if (solution->isSolved) {
        ImGui::Text(l10nc("Optimal value: %.4f"), solution->optimalValue);
        ImGui::Text(l10nc("Optimal plan: %.3fX₁ %.3fX₂ %.3fX₃"), solution->optimalVector.x, solution->optimalVector.y, solution->optimalVector.z);
    } else if (!solution->isErrored && !solution->isSolved && !solution->statusString.empty()) {
        ImGui::Text(l10nc("Solution status: %s"), solution->statusString.c_str());
    }

    #ifdef DEBUG
    ImGui::Checkbox("Show debug overlay (imgui)", &SceneData::showDebugOverlay);
    ImGui::Checkbox("Toggle freecam", &SceneData::canMoveCamera);

    if (SceneData::showDebugOverlay) ImGui::ShowMetricsWindow(&SceneData::showDebugOverlay);
    #endif

    ImGui::End();

    auto iio = ImGui::GetIO();
    if (SceneData::canMoveCamera && !(iio.WantCaptureKeyboard || iio.WantCaptureMouse))
        moveCamera(camera, window, timeStep);

    SceneData::worldOrigin->render(camera->getView(), camera->getProjection());
    SceneData::lppshow->render(camera);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void glfwWindowResizeCallback(GLFWwindow *window, int width, int height) {
    imguiWindowSize.x = width * 0.25;
    imguiWindowSize.y = height;

    imguiWindowPosition.x = width - imguiWindowSize.x;
    // imguiWindowPosition.y = ImGui::GetTextLineHeightWithSpacing(); // height - imguiWindowSize.y;

    int viewWidth = width - imguiWindowSize.x;

    glViewport(0, 0, viewWidth, height);
    SceneData::sceneCamera->changeAspectRatio(viewWidth, height);
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

    int windowWidth = 1280;
    int windowHeight = 720;

    GLFWwindow* mainWindow = glfwCreateWindow(windowWidth, windowHeight, "Linear Programming Problem: Show", nullptr, nullptr);
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
    glViewport(0, 0, windowWidth - imguiWindowSize.x, windowHeight);
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);

    glfwSwapInterval(1);

    ////////////
    //// INIT IMGUI
    ////////////
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ////////////
    //// SCENE SETUP
    ////////////
    LocalMan::updateLocales();
    LocalMan::setToDefault();
    auto* camera = new Camera(windowWidth - imguiWindowSize.x, windowHeight);
    camera->teleportTo(7.35889, 6.92579, 4.95831);
    camera->rotate(-26.4407, 0.0, -133.3081);
    // 7.35889 m, -6.92579 m, 4.95831 m // X, -Y, Z
    // 63.5593°, 0°, 46.6919° // pitch - 90, 0, yaw - 180
    camera->lookDepth = glm::length(camera->getCameraLocation());
    SceneData::sceneCamera = camera;

    try {
        SceneData::lppshow = new Display();
        SceneData::worldOrigin = new WorldGridDisplay();
    } catch (std::exception &ioerr) {
        // Might get to segfault
        return logCriticalError("Failed to compile required shaders");
    }

    // Setup LPP Display with usual x, y, z >= 0 constraints and turn them off
    SceneData::lppshow->objectiveFunction = {0, 0, 0, 0};
    SceneData::lppshow->addLimitPlane({0, 0, 1, 0}, EquationType::GREATER_EQUAL_THAN); SceneData::lppshow->visibleEquations[0] = false;
    SceneData::lppshow->addLimitPlane({0, 1, 0, 0}, EquationType::GREATER_EQUAL_THAN); SceneData::lppshow->visibleEquations[1] = false;
    SceneData::lppshow->addLimitPlane({1, 0, 0, 0}, EquationType::GREATER_EQUAL_THAN); SceneData::lppshow->visibleEquations[2] = false;
    ImGuiIO& iio = ImGui::GetIO(); (void) iio;
    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(iio.Fonts->GetGlyphRangesDefault());
    builder.AddRanges(iio.Fonts->GetGlyphRangesCyrillic());
    builder.AddText("₀₁₂₃₄₅₆₇₈₉");
    builder.BuildRanges(&ranges);

    iio.Fonts->AddFontFromFileTTF("assets/DejaVuSansMono.ttf", 14, nullptr, ranges.Data);
    iio.Fonts->Build();
    #ifndef DEBUG
    iio.IniFilename = NULL;
    #endif

    // ???
    glfwWindowResizeCallback(mainWindow, windowWidth, windowHeight);

    float lastFrame, deltaTime = 0;

    while (!glfwWindowShouldClose(mainWindow)) {
        float time = glfwGetTime();
        deltaTime = time - lastFrame;
        lastFrame = time;

        glClearColor(worldColor.x, worldColor.y, worldColor.z, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /** XXX: The mouse sometimes just keeps its state for a few seconds or even frames
         * if we don't do that. Could be because ImGui takes over and doesn't run the mouse
         * events. We could try using callbacks now that we consider its mouse capture.
         */
        glfwMouseCallback(mainWindow);
        updateProcessDraw(mainWindow, camera, deltaTime);

        glfwSwapBuffers(mainWindow);
        glfwPollEvents();
    }

    // We have to delete them before we deinit glfw and exit the program scope (and consequently opengl)
    // As otherwise we attempt to asl now unloaded GL context to deallocate the object and shader buffers
    delete SceneData::lppshow;
    delete SceneData::worldOrigin;
    glfwTerminate();
}
