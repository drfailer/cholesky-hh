#ifndef CHOLESKY_SOLVER_GRAPH_H
#define CHOLESKY_SOLVER_GRAPH_H
#include "../data/matrix_block_data.h"
#include "../data/matrix_data.h"
#include "../task/split_matrix_task.h"
#include "../task/solver/solve_diagonal_task.h"
#include "../task/solver/update_vector_task.h"
#include "../state/solver/solver_state.h"
#include "../state/solver/solver_state_manager.h"
#include <hedgehog/hedgehog.h>

#define CSGraphInNb 2
#define CSGraphIn MatrixData<T, MatrixTypes::Vector>, MatrixBlockData<T, Result>
#define CSGraphOut MatrixBlockData<T, Result>

template<typename T>
class CholeskySolverGraph
        : public hh::Graph<CSGraphInNb, CSGraphIn, CSGraphOut > {
 public:
  CholeskySolverGraph()
          : hh::Graph<CSGraphInNb, CSGraphIn, CSGraphOut >(
          "Cholesky Decomposition") {
    auto splitMatrixTask = std::make_shared<SplitMatrixTask<T, Block>>();
    auto solveDiagonalTask = std::make_shared<SolveDiagonalTask<T>>(1);
    auto updateVectorTask = std::make_shared<UpdateVectorTask<T>>(10);
    auto solverState = std::make_shared<SolverState<T>>();
    auto solverStateManager = std::make_shared<SolverStateManager<T>>(solverState);

    this->inputs(splitMatrixTask);
    this->inputs(solverStateManager);

    this->edges(splitMatrixTask, solverStateManager);

    this->edges(solverStateManager, solveDiagonalTask);
    this->edges(solveDiagonalTask, solverStateManager);

    this->edges(solverStateManager, updateVectorTask);
    this->edges(updateVectorTask, solverStateManager);

    this->outputs(solverStateManager);
  }
};

#endif
