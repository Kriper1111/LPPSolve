#pragma once

class LinearProgrammingProblem {
    struct Solution
    {
        bool isSolved = false;
        bool isErrored = false;
        bool didMinimize;
        float optimalValue;
        glm::vec4 optimalVector;
        std::string errorString;
        std::vector<float> polyhedraVertices;
    };

    protected:
    std::vector<int> pointlessEquations; // Basically all zeroes
    std::vector<glm::vec4> planeEquations;
    Solution solution;

    void collectPointless();
    virtual void recalculatePlane(int planeIndex) {};
    virtual void onSolutionSolved() {};

    public:
    glm::vec4 objectiveFunction;

    LinearProgrammingProblem();

    virtual int getEquationCount();

    void setObjectiveFunction(glm::vec4 objectiveFunction);
    const glm::vec4 getObjectiveFunction();

    bool doMinimize = true;

    int addLimitPlane(glm::vec4 constraints);
    void editLimitPlane(int planeIndex, glm::vec4 constraints);
    glm::vec4 getLimitPlane(int planeIndex);
    void removeLimitPlane(int planeIndex);
    void removeLimitPlane();

    void solve();

    bool isSolved();
    const Solution* getSolution();

    virtual ~LinearProgrammingProblem();
};

class Display:public LinearProgrammingProblem {
    private:
    static std::shared_ptr<Object> planeObject;
    static std::shared_ptr<Shader> planeShader;
    
    std::shared_ptr<Object> solutionObject;
    std::vector<glm::mat4> planeTransforms;

    static void createPlaneObject();
    static void createPlaneShader();

    void recalculatePlane(int planeIndex);
    void rebindAttributes();
    void onSolutionSolved();

    public:
    std::vector<bool> visibleEquations; // We could maybe merge that into one flag?

    Display();

    int getEquationCount();
    void render(Camera* camera);

    ~Display();
};

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

    std::vector<int> pointlessEquations; // Basically all zeroes
    std::vector<glm::vec4> planeEquations;

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