#pragma once

class LinearProgrammingProblemDisplay {
    struct Solution
    {
        bool isSolved = false;
        bool isErrored = false;
        bool didMinimize;
        float optimalValue;
        glm::vec4 optimalVector;
        std::string errorString;
        std::unique_ptr<Object> object;
    };

    private:
    static std::shared_ptr<Object> planeObject;
    static std::shared_ptr<Shader> planeShader;

    static void createPlaneObject();
    static void createPlaneShader();

    std::vector<int> pointlessEquations; // Basically all zeroes
    std::vector<glm::vec4> planeEquations;
    std::vector<glm::mat4> planeTransforms;

    glm::vec4 objectiveFunction;
    Solution solution;

    void collectPointless();
    void recalculatePlane(int planeIndex);
    void rebindAttributes();

    public:
    std::vector<bool> visibleEquations; // We could maybe merge that into one flag?

    LinearProgrammingProblemDisplay();

    int getEquationCount();

    void setObjectiveFunction(glm::vec4 objectiveFunction);
    const glm::vec4 getObjectiveFunction();

    bool doMinimize = true;

    int addLimitPlane(glm::vec4 constraints);
    int addLimitPlane(float* constraints);
    void editLimitPlane(int planeIndex, glm::vec4 constraints);
    void editLimitPlane(int planeIndex, float* constraints);
    glm::vec4 getLimitPlane(int planeIndex);
    void removeLimitPlane();
    void removeLimitPlane(int planeIndex);

    void solve();

    bool isSolved();
    const Solution* getSolution();
    float getOptimalValue();
    glm::vec4 getOptimalVertex();

    void renderLimitPlanes(glm::mat4 view, glm::mat4 projection);
    void renderAcceptableValues(glm::mat4 view, glm::mat4 projection);
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

    void zoomGrid(float zoomAmount);
    void render(glm::mat4 view, glm::mat4 projection);

    ~WorldGridDisplay();
};