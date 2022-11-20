#include <memory>
#include <vector>

#include "assets.h"
#include "camera.h"
#include "glm/glm.hpp"
#include "solver.h"

#ifdef USE_CDDLIB
#define REFLECT(var) #var
#include <cdd/setoper.h>
#include <cdd/cdd.h>

#include <quickhull/QuickHull.hpp>
#endif

#ifdef USE_BAKED_SHADERS
#include "baked_shaders.h"
#endif

#ifdef USE_CDDLIB
// XXX: This code is ugly and could at least be baked in
const char* reflect_dd_error(dd_ErrorType error) {
    switch (error)
    {
        case dd_DimensionTooLarge: return REFLECT(dd_DimensionTooLarge);
        case dd_ImproperInputFormat: return REFLECT(dd_ImproperInputFormat);
        case dd_NegativeMatrixSize: return REFLECT(dd_NegativeMatrixSize);
        case dd_EmptyVrepresentation: return REFLECT(dd_EmptyVrepresentation);
        case dd_EmptyHrepresentation: return REFLECT(dd_EmptyHrepresentation);
        case dd_EmptyRepresentation: return REFLECT(dd_EmptyRepresentation);
        case dd_IFileNotFound: return REFLECT(dd_IFileNotFound);
        case dd_OFileNotOpen: return REFLECT(dd_OFileNotOpen);
        case dd_NoLPObjective: return REFLECT(dd_NoLPObjective);
        case dd_NoRealNumberSupport: return REFLECT(dd_NoRealNumberSupport);
        case dd_NotAvailForH: return REFLECT(dd_NotAvailForH);
        case dd_NotAvailForV: return REFLECT(dd_NotAvailForV);
        case dd_CannotHandleLinearity: return REFLECT(dd_CannotHandleLinearity);
        case dd_RowIndexOutOfRange: return REFLECT(dd_RowIndexOutOfRange);
        case dd_ColIndexOutOfRange: return REFLECT(dd_ColIndexOutOfRange);
        case dd_LPCycling: return REFLECT(dd_LPCycling);
        case dd_NumericallyInconsistent: return REFLECT(dd_NumericallyInconsistent);
        case dd_NoError: return REFLECT(dd_NoError);
        default: return "Unknown dd_ErrorType";
    }
}

const char* reflect_lp_status(dd_LPStatusType status) {
    switch (status)
    {
        case dd_LPSundecided: return REFLECT(dd_LPSundecided);
        case dd_Optimal: return REFLECT(dd_Optimal);
        case dd_Inconsistent: return REFLECT(dd_Inconsistent);
        case dd_DualInconsistent: return REFLECT(dd_DualInconsistent);
        case dd_StrucInconsistent: return REFLECT(dd_StrucInconsistent);
        case dd_StrucDualInconsistent: return REFLECT(dd_StrucDualInconsistent);
        case dd_Unbounded: return REFLECT(dd_Unbounded);
        case dd_DualUnbounded: return REFLECT(dd_DualUnbounded);
        default: return "Unknown dd_LPStatusType";
    }
}

// @throws std::runtime_error if there's a dd error
void throw_dd_error(dd_ErrorType error) noexcept(false) {
    if (error != dd_NoError) {
        throw std::runtime_error(reflect_dd_error(error));
    }
}

glm::vec4 createVector(dd_Arow dd_vector, int vector_length) {
    glm::vec4 vector(0);
    for (int index = 0; index < vector_length && index < 4; index++) {
        vector[index] = dd_vector[index][0];
    }
    return vector;
}

std::vector<float> getVertices(dd_MatrixPtr vform) {
    std::vector<float> vertices;
    for (int row = 0; row < vform->rowsize; row++) {
        if (vform->matrix[row][0][0] == 0) continue;
        vertices.push_back(vform->matrix[row][1][0]);
        vertices.push_back(vform->matrix[row][2][0]);
        vertices.push_back(vform->matrix[row][3][0]);
    }
    return vertices;
}

std::vector<std::vector<int>> getAdjacency(dd_SetFamilyPtr adj) {
    std::vector<std::vector<int>> adjacency;
    for (int vertex = 0; vertex < adj->famsize; vertex++) {
        std::vector<int> vertex_adjacent;

		long cardinality = set_card(adj->set[vertex]);
		// bool invert = adj->setsize - cardinality >= cardinality;
        if (cardinality == adj->famsize) continue;
		for (int elemt = 1; elemt <= adj->set[vertex][0]; elemt++) {
			if (set_member(elemt, adj->set[vertex]))
				vertex_adjacent.push_back(elemt);
		}

        // XXX: May cause some issues, if an empty row will show up first.
        if (!vertex_adjacent.empty())
            adjacency.push_back(vertex_adjacent);
    }
    return adjacency;
}

#endif // USE_CDDLIB

/**
 * FIXME: This whole thing isn't thread-safe. At all. Which is a candidate
 * to a change, don't want to hang rendering for too long with calculations.
*/
// class LinearProgrammingProblem {
// protected:

void LinearProgrammingProblem::collectPointless() {
    // It never gets better, does it?
    // I hope they're sorted
    for (const int index : this->pointlessEquations)
        this->removeLimitPlane(index);
    this->pointlessEquations.clear();
}

//  public:

LinearProgrammingProblem::LinearProgrammingProblem() {
    this->planeEquations = std::vector<glm::vec4>();
    this->pointlessEquations = std::vector<int>();
};
LinearProgrammingProblem::~LinearProgrammingProblem() {
    this->planeEquations.clear();
    this->pointlessEquations.clear();
};

int LinearProgrammingProblem::getEquationCount() {
    this->collectPointless();
    return planeEquations.size();
}

int LinearProgrammingProblem::addLimitPlane(glm::vec4 constraints) {
    if (constraints.x != 0 || constraints.y != 0 || constraints.z != 0 || constraints.w != 0) {
        planeEquations.push_back(constraints);
        // recalculatePlane(planeEquations.size() - 1);
        onPlaneAdded(planeEquations.size() - 1);
    }
    return planeEquations.size();
}

// @throws: std::out_of_range
glm::vec4 LinearProgrammingProblem::getLimitPlane(int planeIndex) { return planeEquations.at(planeIndex); }

void LinearProgrammingProblem::editLimitPlane(int planeIndex, glm::vec4 constraints) {
    planeEquations[planeIndex] = constraints;
    onPlaneUpdated(planeIndex);
    // recalculatePlane(planeIndex);
    if (constraints.x == 0 && constraints.y == 0 && constraints.z == 0 && constraints.w == 0)
        this->pointlessEquations.push_back(planeIndex);
}

void LinearProgrammingProblem::removeLimitPlane() {
    planeEquations.pop_back();
    onPlaneRemoved(planeEquations.size());
}
void LinearProgrammingProblem::removeLimitPlane(int planeIndex) {
    if (planeIndex < 0 || planeIndex >= planeEquations.size()) return;
    planeEquations.erase(planeEquations.begin() + planeIndex);
    onPlaneRemoved(planeIndex);
}

template <typename dd_Type>
using dd_unique_ptr = std::unique_ptr<dd_Type, void(*)(dd_Type*)>;

/** 
 * Solves the given LPP and return boolean if a solution was found.
 * If the provided system is invalid, don't throw but set solution.isSolved to false
 * Query solution.statusString for details.
 * 
 * NOTE: with dd_unique_ptr wrapper it's less likely to throw a segfault than plainly..
 *      deleting them after an exception is caught/scope exited, but who knows.
 *      This approach relies on RAII to deinit the values, which shouldn't happen if the..
 *      stored pointer is null, like the initial state.
 *      Basically saves me from writing `if (polyhedra != nullptr) dd_FreePolyhedra(polyhedra)`
 *      I'd much rather write this comment :P
 * @throws std::runtime_error if there's something really wrong with the provided system
 */
void LinearProgrammingProblem::solve() {
    this->collectPointless();
    // Yes we use #ifdef and I know it's bad, but I have to build it somehow on Windows first.
    #ifdef USE_CDDLIB
    /**
     */
    dd_unique_ptr<dd_LPType>    linearProgrammingProblem(nullptr, dd_FreeLPData);
    dd_unique_ptr<dd_MatrixType> constraintMatrix(nullptr, dd_FreeMatrix);
    dd_unique_ptr<dd_MatrixType> verticesMatrix(nullptr, dd_FreeMatrix);
    dd_unique_ptr<dd_SetFamilyType> adjacency(nullptr, dd_FreeSetFamily);
    dd_unique_ptr<dd_PolyhedraType> polyhedra(nullptr, dd_FreePolyhedra);
    dd_ErrorType error;
    try {
    dd_set_global_constants();

    solution = Solution();

    constraintMatrix.reset(dd_CreateMatrix(3 + this->planeEquations.size(), 4));

    int row;
    for (row = 0; row < planeEquations.size(); row++) {
        // Ah yes I love doing stuff this way. Just can't get enough of it.
        // *sarcarsm please don't judge*
        /** XXX: cddlib expects us to provide the constraints in a different form
         * it expects the form of:
         * B A1 A2 A3 >= 0
         * but we collect them in form of
         * A1 A2 A3 <= B
         * so we have to multiply A's (planeEquation.xyz) by -1
         * And shift B (planeEquation.w) to the first column.
         */
        const auto planeEquation = planeEquations.at(row);
        dd_set_d(constraintMatrix->matrix[row][0],  planeEquation.w);
        dd_set_d(constraintMatrix->matrix[row][1], -planeEquation.x);
        dd_set_d(constraintMatrix->matrix[row][2], -planeEquation.y);
        dd_set_d(constraintMatrix->matrix[row][3], -planeEquation.z);
    }
    for (int diag = 0; diag < 3; diag++) {
        // -This way we set the x1, x2, x3 >= 0 condition
        // -Other form is
        // dd_set_d(constraintMatrix->matrix[row + 0][1], 1.0);
        // dd_set_d(constraintMatrix->matrix[row + 1][2], 1.0);
        // dd_set_d(constraintMatrix->matrix[row + 2][3], 1.0);
        //  without the loop
        dd_set_d(constraintMatrix->matrix[row + diag][diag + 1], 1.0);
    }

    // For some reason we don't need to invert the objective function?
    dd_set_d(constraintMatrix->rowvec[0], objectiveFunction.w);
    dd_set_d(constraintMatrix->rowvec[1], objectiveFunction.x);
    dd_set_d(constraintMatrix->rowvec[2], objectiveFunction.y);
    dd_set_d(constraintMatrix->rowvec[3], objectiveFunction.z);

    constraintMatrix->representation = dd_Inequality;
    constraintMatrix->objective = this->doMinimize ? dd_LPmin : dd_LPmax;

    linearProgrammingProblem.reset(dd_Matrix2LP(constraintMatrix.get(), &error));
    throw_dd_error(error);
    polyhedra.reset(dd_DDMatrix2Poly(constraintMatrix.get(), &error));
    throw_dd_error(error);
    dd_LPSolve(linearProgrammingProblem.get(), dd_DualSimplex, &error);
    throw_dd_error(error);

    verticesMatrix.reset(dd_CopyGenerators(polyhedra.get()));
    adjacency.reset(dd_CopyAdjacency(polyhedra.get()));

    solution.isSolved = linearProgrammingProblem->LPS == dd_LPStatusType::dd_Optimal;
    solution.statusString = reflect_lp_status(linearProgrammingProblem->LPS);
    solution.optimalValue = linearProgrammingProblem->optvalue[0];
    solution.optimalVector = createVector(linearProgrammingProblem->sol, linearProgrammingProblem->d);
    solution.didMinimize = linearProgrammingProblem->objective == dd_LPmin;
    solution.polyhedraVertices = getVertices(verticesMatrix.get());
    solution.adjacency = getAdjacency(adjacency.get());
    onSolutionSolved();

    } catch (std::runtime_error &dd_error) {
        dd_free_global_constants();
        throw dd_error;
    }
    dd_free_global_constants();
    #endif
};

const LinearProgrammingProblem::Solution* LinearProgrammingProblem::getSolution() {
    return &this->solution;
}