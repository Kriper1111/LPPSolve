#pragma once

enum EquationType {
    LESS_EQUAL_THAN = 0,
    GREATER_EQUAL_THAN = 1,
    EQUAL_TO = 2
};
// const char* getEquationType(EquationType equationType) {
//     switch (equationType)
//     {
//     case EquationType::LESS_EQUAL_THAN: return "<=";
//     case EquationType::GREATER_EQUAL_THAN: return ">=";
//     case EquationType::EQUAL_TO: return "=";
//     }
//     return "";
// }
class LinearProgrammingProblem {
    private:
    struct Equation {
        glm::vec4 equationCoefficients;
        EquationType type;
    };
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
    std::vector<Equation> planeEquations;
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
    int addLimitPlane(glm::vec4 constraints, EquationType equationType);
    int addLimitPlane(Equation equation);
    Equation getLimitPlane(int planeIndex);
    void editLimitPlane(int planeIndex, glm::vec4 constraints);
    void editLimitPlane(int planeIndex, glm::vec4 constraints, EquationType equationType);
    void editLimitPlane(Equation equation);
    void removeLimitPlane();
    void removeLimitPlane(int planeIndex);
    void reset();

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
    glm::mat4 globalScaleTransform = glm::mat4(1);

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
    double globalScale = 1.0;
    float stripeFrequency = 10.0;
    float stripeWidth = 0.5;
    float wireThickness = 1.0;
    float vectorWidth = 0.1;
    float arrowScale = 2.5;

    std::vector<glm::vec3> constraintPositiveColors = {
        {0.9216, 0.2863, 0.4627},
        {0.1922, 0.3647, 0.9216},
        {0.9216, 0.6980, 0.1020},
        {0.1490, 0.9216, 0.2431}
    };
    glm::vec3 solutionColor = {1.0, 0.746282, 0.043526};
    glm::vec3 solutionVectorColor = { 0, 0, 0 }; // {0.128, 0.833, 0.272};
    glm::vec3 solutionWireframeColor = {0.8, 0.095672, 0.019807};

    Display();

    void setScale(double scale);
    void render(Camera* camera);

    ~Display();
};

class WorldGridDisplay {
    private:
    std::shared_ptr<Object> gridObject;
    std::shared_ptr<Object> axisObject;
    std::shared_ptr<Shader> gridShader;
    std::shared_ptr<Shader> axisShader;
    int scaleExponent = 0;

    void createObjects();
    void createShaders();

    public:
    bool gridEnabled = true;
    bool axisEnabled = true;

    float gridScale = 1.0;
    float gridWidth = 0.05;

    WorldGridDisplay();

    void zoomGrid(float zoomAmount);
    double getComputedScale();
    void render(glm::mat4 view, glm::mat4 projection);

    ~WorldGridDisplay();
};
#endif
