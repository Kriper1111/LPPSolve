#include <iostream>
#include <memory>
#include <vector>
// XXX : We shouldn't have to include them in each implementation for the solver to work.
// Forward-declare them maybe?
#include "assets.h"
#include "camera.h"

#include <cstdarg>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "solver.h"

using std::cout;
using std::cerr;
using std::endl;

bool solver_sanity_check() {
    const glm::vec4 trialVector = {1, 1, 1, 0};
    const glm::vec4 editedVector = {1, 2, 1, 2};
    std::unique_ptr<LinearProgrammingProblem> solver = std::make_unique<LinearProgrammingProblem>();
    solver->addLimitPlane({1, 1, 1, 0});
    if (solver->getEquationCount() != 1) return false;
    if (solver->getLimitPlane(0) != trialVector) return false;
    solver->editLimitPlane(0, glm::vec4({1, 2, 1, 2}));
    if (solver->getLimitPlane(0) != editedVector) return false;
    solver->removeLimitPlane();
    if (solver->getEquationCount() != 0) return false;
    return true;
}

bool solver_invalid_solution() {
    std::unique_ptr<LinearProgrammingProblem> solver = std::make_unique<LinearProgrammingProblem>();

    const glm::vec4 constraintOne = {  1, 0, 0,  5 };  // x <= 5
    const glm::vec4 constraintTwo = { -1, 0, 0, -7 };  // -x <= -7 -> x >= 7
    const glm::vec4 objectiveFunction = { 1, 1, 0, 0}; // top-left

    solver->objectiveFunction = objectiveFunction;
    solver->doMinimize = false;
    solver->addLimitPlane(constraintOne);
    solver->addLimitPlane(constraintTwo);

    solver->solve();
    return !solver->getSolution()->isSolved;
}

bool solver_2d_solution_max() {
    std::unique_ptr<LinearProgrammingProblem> solver = std::make_unique<LinearProgrammingProblem>();

    const glm::vec4 objectiveFunction = { 3, 4, 0, 0 }; // top-left-ish
    const glm::vec4 constraintOne = {  1, 0, 0, 1 };  // x <= 1
    const glm::vec4 constraintTwo = {  0, 1, 0, 1 };  // y <= 1

    solver->objectiveFunction = objectiveFunction;
    solver->doMinimize = false;
    solver->addLimitPlane(constraintOne);
    solver->addLimitPlane(constraintTwo);

    solver->solve();
    return solver->getSolution()->optimalValue == 7;
}

bool solver_2d_solution_min() {
    std::unique_ptr<LinearProgrammingProblem> solver = std::make_unique<LinearProgrammingProblem>();

    const glm::vec4 objectiveFunction = {  3, 4, 0, 0 }; // 3x + 4y -> min
    const glm::vec4 constraintOne    =  {  1, 0, 0, 1 }; //  x      <= 1
    const glm::vec4 constraintTwo    =  {  0, 1, 0, 1 }; //       y <= 1
    const glm::vec4 constraintThree  =  { -1,-2, 0,-1 }; //  x + 2y >= 1

    solver->objectiveFunction = objectiveFunction;
    solver->doMinimize = true;
    solver->addLimitPlane(constraintOne);
    solver->addLimitPlane(constraintTwo);
    solver->addLimitPlane(constraintThree);

    solver->solve();
    return solver->getSolution()->optimalValue == 2;
}

// XXX: We'll test the default cube for the lack of anything else
bool solver_3d_solution() {
    std::unique_ptr<LinearProgrammingProblem> solver = std::make_unique<LinearProgrammingProblem>();

    const glm::vec4 objectiveFunction = {  3, 3, 2, 0 }; // 3x + 3y + 2z -> min
    const glm::vec4 constraintOne    =  {  1, 0, 0, 1 }; //  x           <= 1
    const glm::vec4 constraintTwo    =  {  0, 1, 0, 1 }; //       y      <= 1
    const glm::vec4 constraintThree  =  {  0, 0, 1, 1 }; //            z <= 1

    solver->objectiveFunction = objectiveFunction;
    solver->doMinimize = false;
    solver->addLimitPlane(constraintOne);
    solver->addLimitPlane(constraintTwo);
    solver->addLimitPlane(constraintThree);

    solver->solve();
    return solver->getSolution()->optimalValue == 8;
}

bool solver_2d_vertices() {
    std::unique_ptr<LinearProgrammingProblem> solver = std::make_unique<LinearProgrammingProblem>();

    const glm::vec4 objectiveFunction = {  3,  3,  0,   0 }; // 3x + 3y -> min
    const glm::vec4 constraintOne    =  { -1, -2,  0,  -4 }; //  x + 2y >=   4
    const glm::vec4 constraintTwo    =  { -5, -1,  0, -11 }; // 5x +  y >=  11
    const glm::vec4 constraintThree  =  { -1,  4,  0,  23 }; //  x - 4y >= -23
    const glm::vec4 constraintFour   =  {  4,  5,  0,  55 }; //-4x - 5y >= -55

    std::vector<glm::vec3> vertices = {
        {    1, 6, 0},
        {    5, 7, 0},
        {13.75, 0, 0},
        {    4, 0, 0},
        {    2, 1, 0}
    };

    solver->objectiveFunction = objectiveFunction;
    solver->doMinimize = false; // Irrelevant, really
    solver->addLimitPlane(constraintOne);
    solver->addLimitPlane(constraintTwo);
    solver->addLimitPlane(constraintThree);
    solver->addLimitPlane(constraintFour);

    solver->solve();

    if (!solver->getSolution()->isSolved) return false;
    auto solutionVertices = solver->getSolution()->polyhedraVertices;
    if (solutionVertices.size() != 5 * 3) return false;

    for (int vtx = 0; vtx < solutionVertices.size(); vtx += 3) {
        auto vector = glm::vec3(solutionVertices[vtx], solutionVertices[vtx+1], solutionVertices[vtx+2]);
        int found = -1;
        for (int index = 0; index < vertices.size(); index++) {
            if (vertices[index] != vector) continue;
            found = index;
            break;
        }
        if (found == -1) return false;
        vertices.erase(vertices.begin() + found);
    }

    return true;
}

// Again, borrowing le cube for the time being
bool solver_3d_vertices() {
    std::unique_ptr<LinearProgrammingProblem> solver = std::make_unique<LinearProgrammingProblem>();

    const glm::vec4 objectiveFunction = {  3,  3,  0,  1 }; // 3x + 3y      -> min
    const glm::vec4 constraintOne    =  {  1,  0,  0,  1 }; //  x           <= 1
    const glm::vec4 constraintTwo    =  {  0,  1,  0,  1 }; //       y      <= 1
    const glm::vec4 constraintThree  =  {  0,  0,  1,  1 }; //            z <= 1

    std::vector<glm::vec3> vertices = {
        { 0, 0, 0 },
        { 1, 1, 0 },
        { 0, 1, 1 },
        { 1, 0, 1 },
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 1 },
        { 1, 1, 1 }
    };

    solver->objectiveFunction = objectiveFunction;
    solver->doMinimize = false; // Irrelevant, really
    solver->addLimitPlane(constraintOne);
    solver->addLimitPlane(constraintTwo);
    solver->addLimitPlane(constraintThree);

    solver->solve();

    if (!solver->getSolution()->isSolved) return false;
    auto solutionVertices = solver->getSolution()->polyhedraVertices;
    if (solutionVertices.size() != 8 * 3) return false;

    for (int vtx = 0; vtx < solutionVertices.size(); vtx += 3) {
        auto vector = glm::vec3(solutionVertices[vtx], solutionVertices[vtx+1], solutionVertices[vtx+2]);
        int found = -1;
        for (int index = 0; index < vertices.size(); index++) {
            if (vertices[index] != vector) continue;
            found = index;
            break;
        }
        if (found == -1) return false;
        vertices.erase(vertices.begin() + found);
    }

    return true;
}

bool solver_vertices_invalid() {
    std::unique_ptr<LinearProgrammingProblem> solver = std::make_unique<LinearProgrammingProblem>();

    const glm::vec4 constraintOne = {  1, 0, 0,  5 };  // x <= 5
    const glm::vec4 constraintTwo = { -1, 0, 0, -7 };  // -x <= -7 -> x >= 7
    const glm::vec4 objectiveFunction = { 1, 1, 0, 0}; // top-left

    solver->objectiveFunction = objectiveFunction;
    solver->doMinimize = false;
    solver->addLimitPlane(constraintOne);
    solver->addLimitPlane(constraintTwo);

    solver->solve();
    return solver->getSolution()->polyhedraVertices.size() == 0;
}

typedef bool test();

bool run_test(test* test_function, const char* test_name) {
    cout << "Running " << test_name << "... ";
    bool result = false;
    try { result = test_function(); }
    catch (...) {}
    cout << (result ? "passed" : "failed") << endl;
    return result;
}

int main() {
    cout << "Performing tests" << endl;

    int passed = 0;
    int failed = 0;

    try {
    run_test(solver_sanity_check, "Solver: Sanity check") ? passed += 1 : failed += 1;
    run_test(solver_invalid_solution, "Solver: With invalid input") ? passed += 1 : failed += 1;
    run_test(solver_2d_solution_min, "Solver: 2D solution -> min") ? passed += 1 : failed += 1;
    run_test(solver_2d_solution_max, "Solver: 2D solution -> max") ? passed += 1 : failed += 1;
    run_test(solver_3d_solution, "Solver: 3D solution") ? passed += 1 : failed += 1;
    run_test(solver_2d_vertices, "Solver: 2D Extreme points") ? passed += 1 : failed += 1;
    run_test(solver_3d_vertices, "Solver: 3D Extreme points") ? passed += 1 : failed += 1;
    run_test(solver_vertices_invalid, "Solver: Extreme points with invalid system") ? passed += 1 : failed += 1;
    } catch (std::runtime_error &err) {
        cerr << "How unexpected, the test suite ran into a problem" << endl;
        cerr << "err.what(): " << err.what() << endl;
        cerr << "We're sorry." << endl;
    }

    cout << "Tests completed" << endl;
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    return failed;
}
