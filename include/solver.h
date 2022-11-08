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
        std::string statusString;
        std::vector<float> polyhedraVertices;
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

class Display:public LinearProgrammingProblem {
    private:
    std::shared_ptr<Object> planeObject;
    std::shared_ptr<Shader> planeShader;
    
    std::shared_ptr<Object> solutionObject;
    std::vector<glm::mat4> planeTransforms;

    void createPlaneObject();
    void createPlaneShader();

    void recalculatePlane(int planeIndex);
    void rebindAttributes();
    void onSolutionSolved();
    void onPlaneAdded(int planeIndex);
    void onPlaneUpdated(int planeIndex);
    void onPlaneRemoved(int planeIndex);

    public:
    std::vector<bool> visibleEquations; // We could maybe merge that into one flag?
    bool showPlanesAtAll;

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
    bool gridEnabled = false;
    bool axisEnabled = true;

    float zoomScale = 1.0;

    WorldGridDisplay();

    void zoomGrid(float zoomAmount);
    void render(glm::mat4 view, glm::mat4 projection);

    ~WorldGridDisplay();
};