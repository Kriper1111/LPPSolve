#pragma once

// #include <glm/glm.hpp>

class LinearProgrammingProblemDisplay {
    private:
    static std::shared_ptr<Object> planeObject;
    static std::shared_ptr<Shader> planeShader;

    static void createPlaneObject();
    static void createPlaneShader();

    std::vector<glm::vec4> planeEquations;
    std::vector<glm::mat4> planeTransforms;

    void recalculatePlane(int planeIndex);
    void rebindAttributes();

    public:
    LinearProgrammingProblemDisplay();

    int getEquationCount();

    int addLimitPlane(glm::vec4 constraints);
    int addLimitPlane(float* constraints);
    void editLimitPlane(int planeIndex, glm::vec4 constraints);
    void editLimitPlane(int planeIndex, float* constraints);
    glm::vec4 getLimitPlane(int planeIndex);
    void removeLimitPlane();
    void removeLimitPlane(int planeIndex);

    bool solve();

    void renderLimitPlanes(glm::mat4 view, glm::mat4 projection);
    void renderAcceptableValues();
    void renderSolution();

    ~LinearProgrammingProblemDisplay();
};

class WorldGridDisplay {
    private:
    static std::shared_ptr<Object> gridObject;
    static std::shared_ptr<Object> axisObject;
    static std::shared_ptr<Shader> gridShader;
    static std::shared_ptr<Shader> axisShader;

    static void createObjects();
    static void createShaders();

    public:
    bool gridEnabled = false;
    bool axisEnabled = true;

    float zoomScale = 1.0;

    WorldGridDisplay();

    void render(glm::mat4 view, glm::mat4 projection);

    ~WorldGridDisplay();
};