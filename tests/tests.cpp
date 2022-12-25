#include <iostream>
#include <memory>
#include <vector>

#include <cstdarg>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "solver.h"
#define LOCALMAN_IMPL
#include "localman.h"

#define test(function, name) run_test(function, name) ? passed+=1 : failed+=1

using std::cout;
using std::cerr;
using std::endl;

bool solver_sanity_check() {
    const glm::vec4 trialVector = {1, 1, 1, 0};
    const glm::vec4 editedVector = {1, 2, 1, 2};
    std::unique_ptr<LinearProgrammingProblem> solver = std::make_unique<LinearProgrammingProblem>();
    solver->addLimitPlane({1, 1, 1, 0});
    if (solver->getEquationCount() != 1) return false;
    if (solver->getLimitPlane(0).equationCoefficients != trialVector) return false;
    solver->editLimitPlane(0, glm::vec4({1, 2, 1, 2}));
    if (solver->getLimitPlane(0).equationCoefficients != editedVector) return false;
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
    solver->addLimitPlane({0, 1, 0, 0}, EquationType::GREATER_EQUAL_THAN);
    solver->addLimitPlane({1, 0, 0, 0}, EquationType::GREATER_EQUAL_THAN);

    const glm::vec4 objectiveFunction = { 3, 4, 0, 0 }; // top-left-ish
    const glm::vec4 constraintOne = {  1, 0, 0, 1 };  // x <= 1
    const glm::vec4 constraintTwo = {  0, 1, 0, 1 };  // y <= 1

    solver->objectiveFunction = objectiveFunction;
    solver->doMinimize = false;
    solver->addLimitPlane(constraintOne);
    solver->addLimitPlane(constraintTwo);

    solver->solve();
    return (solver->getSolution()->optimalValue == 7) && (solver->getSolution()->optimalVector == glm::vec3({1, 1, 0}));
}

bool solver_2d_solution_min() {
    std::unique_ptr<LinearProgrammingProblem> solver = std::make_unique<LinearProgrammingProblem>();
    solver->addLimitPlane({1, 0, 0, 0}, EquationType::GREATER_EQUAL_THAN);
    solver->addLimitPlane({0, 1, 0, 0}, EquationType::GREATER_EQUAL_THAN);

    const glm::vec4 objectiveFunction = {  3, 4, 0, 0 }; // 3x + 4y -> min
    const glm::vec4 constraintOne    =  {  1, 0, 0, 1 }; //  x      <= 1
    const glm::vec4 constraintTwo    =  {  0, 1, 0, 1 }; //       y <= 1
    const glm::vec4 constraintThree  =  {  1, 2, 0, 1 }; //  x + 2y >= 1

    solver->objectiveFunction = objectiveFunction;
    solver->doMinimize = true;
    solver->addLimitPlane(constraintOne);
    solver->addLimitPlane(constraintTwo);
    solver->addLimitPlane(constraintThree, EquationType::GREATER_EQUAL_THAN);

    solver->solve();
    return (solver->getSolution()->optimalValue == 2) && (solver->getSolution()->optimalVector == glm::vec3({0, 0.5, 0}));
}

bool solver_2d_solution_equals() {
    std::unique_ptr<LinearProgrammingProblem> solver = std::make_unique<LinearProgrammingProblem>();
    solver->addLimitPlane({0, 1, 0, 0}, EquationType::GREATER_EQUAL_THAN);
    solver->addLimitPlane({1, 0, 0, 0}, EquationType::GREATER_EQUAL_THAN);

    const glm::vec4 objectiveFunction = {  2, 2, 0, 0 }; // 3x + 4y -> max
    const glm::vec4 constraintOne    =  {  1,-3, 0,-7 }; //  x - 3y =  7
    const glm::vec4 constraintTwo    =  { -3, 1, 0,-7 }; //-3x +  y >= 7

    solver->objectiveFunction = objectiveFunction;
    solver->doMinimize = false;
    solver->addLimitPlane(constraintOne, EquationType::EQUAL_TO);
    solver->addLimitPlane(constraintTwo, EquationType::GREATER_EQUAL_THAN);

    solver->solve();
    return (solver->getSolution()->optimalValue == 14) && (solver->getSolution()->optimalVector == glm::vec3({3.5, 3.5, 0}));
}

// XXX: We'll test the default cube for the lack of anything else
bool solver_3d_solution() {
    std::unique_ptr<LinearProgrammingProblem> solver = std::make_unique<LinearProgrammingProblem>();
    solver->addLimitPlane({1, 0, 0, 0}, EquationType::GREATER_EQUAL_THAN);
    solver->addLimitPlane({0, 1, 0, 0}, EquationType::GREATER_EQUAL_THAN);
    solver->addLimitPlane({0, 0, 1, 0}, EquationType::GREATER_EQUAL_THAN);

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
    return (solver->getSolution()->optimalValue == 8) && (solver->getSolution()->optimalVector == glm::vec3({1, 1, 1}));
}

bool solver_2d_vertices() {
    std::unique_ptr<LinearProgrammingProblem> solver = std::make_unique<LinearProgrammingProblem>();
    solver->addLimitPlane({1, 0, 0, 0}, EquationType::GREATER_EQUAL_THAN);
    solver->addLimitPlane({0, 1, 0, 0}, EquationType::GREATER_EQUAL_THAN);

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
    solver->addLimitPlane({0, 0, 1, 0}, EquationType::GREATER_EQUAL_THAN);
    solver->addLimitPlane({0, 1, 0, 0}, EquationType::GREATER_EQUAL_THAN);
    solver->addLimitPlane({1, 0, 0, 0}, EquationType::GREATER_EQUAL_THAN);

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
    solver->addLimitPlane({0, 1, 0, 0}, EquationType::GREATER_EQUAL_THAN);
    solver->addLimitPlane({1, 0, 0, 0}, EquationType::GREATER_EQUAL_THAN);

    const glm::vec4 constraintOne = {  1, 0, 0, 5 };  // x <= 5
    const glm::vec4 constraintTwo = {  1, 0, 0, 7 };  // x >= 7
    const glm::vec4 objectiveFunction = { 1, 1, 0, 0}; // top-left

    solver->objectiveFunction = objectiveFunction;
    solver->doMinimize = false;
    solver->addLimitPlane(constraintOne);
    solver->addLimitPlane(constraintTwo, EquationType::GREATER_EQUAL_THAN);

    solver->solve();
    return solver->getSolution()->polyhedraVertices.size() == 0;
}

bool localman_parse_locale_plain() {
    #ifdef _WIN32
    const char* test_string = "English_United States";
    #else
    const char* test_string = "en_US";
    #endif
    auto locale = LocalMan::getLocale(test_string);
    return (locale.language == "en" && locale.country == "US");
}

bool localman_parse_locale_fluff() {
    #ifdef _WIN32
    // It's very hard to come by the locale name list on Windows
    const char* test_string = "Danish_Denmark.Windows-1252";
    #else
    const char* test_string = "da_DK.ISO8859-15@euro";
    #endif
    auto locale = LocalMan::getLocale(test_string);
    return (locale.language == "da" && locale.country == "DK");
}

bool localman_parse_locale_short() {
    #ifdef _WIN32
    const char* test_string = "Russian";
    #else
    const char* test_string = "ru";
    #endif
    auto locale = LocalMan::getLocale(test_string);
    return (locale.language == "ru" && locale.country == "");
}

bool localman_parse_locale_country_only() {
    #ifdef _WIN32
    const char* test_string = "_Spain";
    #else
    const char* test_string = "_ES";
    #endif
    auto locale = LocalMan::getLocale("_ES");
    return (locale.language == "" && locale.country == "ES");
}

typedef bool TestType();

bool run_test(TestType* test_function, const char* test_name) {
    cout << "Running " << test_name << "... ";
    bool result = false;
    try { result = test_function(); }
    catch (...) {}
    cout << (result ? "\033[32;1mpassed" : "\033[31;1mfailed") << "\033[0;0m" << endl;
    return result;
}

int main() {
    cout << "Performing tests" << endl;
    cout << "===========================\n";

    int passed = 0;
    int failed = 0;

    try {
    test(solver_sanity_check, "Solver: Sanity check");
    test(solver_invalid_solution, "Solver: With invalid input");
    test(solver_2d_solution_min, "Solver: 2D solution -> min");
    test(solver_2d_solution_max, "Solver: 2D solution -> max");
    test(solver_2d_solution_equals, "Solver: 2D solution with = and >=");
    test(solver_3d_solution, "Solver: 3D solution");
    test(solver_2d_vertices, "Solver: 2D Extreme points");
    test(solver_3d_vertices, "Solver: 3D Extreme points");
    test(solver_vertices_invalid, "Solver: Extreme points with invalid system");

    test(localman_parse_locale_plain, "LocalMan: Parse plain locale");
    test(localman_parse_locale_short, "LocalMan: Parse short locale");
    test(localman_parse_locale_country_only, "LocalMan: Parse country only");
    test(localman_parse_locale_fluff, "LocalMan: Parse with extra fluff");
    } catch (std::runtime_error &err) {
        cerr << "How unexpected, the test suite ran into a problem" << endl;
        cerr << "err.what(): " << err.what() << endl;
        cerr << "We're sorry." << endl;
    }

    cout << "===========================\n";
    cout << "Tests completed" << endl;
    cout << "Passed: " << passed << endl;
    cout << "Failed: " << failed << endl;
    return failed;
}
