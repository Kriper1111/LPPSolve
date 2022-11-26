#pragma once

class LinearProgrammingProblem {
    struct Solution
    {
        bool isSolved = false;
        bool isErrored = false;
        bool didMinimize;
        float optimalValue;
        glm::vec3 optimalVector;
        std::string errorString;
        std::string statusString;
        std::vector<float> polyhedraVertices;
        std::vector<std::vector<int>> adjacency;
    };

    protected:
    std::vector<int> pointlessEquations; // Basically all zeroes
    std::vector<glm::vec4> planeEquations;
    Solution solution;

    void collectPointless();

    // virtual "events" for Display compatibility
    virtual void onSolutionSolved() {};
    virtual void onPlaneAdded(int planeIndex) {};
    virtual void onPlaneUpdated(int planeIndex) {};
    virtual void onPlaneRemoved(int planeIndex) {};

    public:
    glm::vec4 objectiveFunction;
    bool doMinimize = true;

    LinearProgrammingProblem();

    int getEquationCount();

    int addLimitPlane(glm::vec4 constraints);
    glm::vec4 getLimitPlane(int planeIndex);
    void editLimitPlane(int planeIndex, glm::vec4 constraints);
    void removeLimitPlane();
    void removeLimitPlane(int planeIndex);

    void solve();

    bool isSolved();
    const Solution* getSolution();

    virtual ~LinearProgrammingProblem();
};

#ifdef DISPLAY_IMPL
class Display:public LinearProgrammingProblem {
    private:
    std::shared_ptr<Object> planeObject;
    std::shared_ptr<Shader> planeShader;

    std::shared_ptr<Object> vectorDisplay;

    std::shared_ptr<Shader> solutionShader;
    std::shared_ptr<Object> solutionObject;
    std::shared_ptr<Object> solutionWireframe;
    std::vector<glm::mat4> planeTransforms;

    glm::mat4 optimalPlanTransform;

    void createObjects();
    void createShaders();

    void recalculatePlane(int planeIndex);
    void recalculateOptimalPlan();
    void rebindAttributes();
    void onSolutionSolved();
    void onPlaneAdded(int planeIndex);
    void onPlaneUpdated(int planeIndex);
    void onPlaneRemoved(int planeIndex);

    public:
    std::vector<bool> visibleEquations; // We could maybe merge that into one flag?
    bool showPlanesAtAll = true;
    bool showSolutionVolume = true;
    bool showSolutionVector = true;
    bool showSolutionWireframe = true;
    float stripeFrequency = 10.0;
    float stripeWidth = 0.5;
    float wireThickness = 1.0;
    float vectorWidth = 0.1;
    float arrowScale = 2.5;

    glm::vec3 constraintPositiveColor = {0.0, 0.0, 1.0};
    glm::vec3 constraintNegativeColor = {1.0, 0.0, 0.0};
    glm::vec3 solutionColor = {1.0, 0.746282, 0.043526};
    glm::vec3 solutionVectorColor = { 0, 0, 0 }; // {0.128, 0.833, 0.272};
    glm::vec3 solutionWireframeColor = {0.8, 0.095672, 0.019807};

    Display();

    void render(Camera* camera);

    ~Display();
};

class WorldGridDisplay {
    private:
    std::shared_ptr<Object> gridObject;
    std::shared_ptr<Object> axisObject;
    std::shared_ptr<Shader> gridShader;
    std::shared_ptr<Shader> axisShader;

    void createObjects();
    void createShaders();

    public:
    bool gridEnabled = true;
    bool axisEnabled = true;

    float gridScale = 1.0;
    float gridWidth = 0.05;

    WorldGridDisplay();

    void zoomGrid(float zoomAmount);
    void render(glm::mat4 view, glm::mat4 projection);

    ~WorldGridDisplay();
};
#endif
