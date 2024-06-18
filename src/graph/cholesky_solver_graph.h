#ifndef CHOLESKY_SOLVER_GRAPH_H
#define CHOLESKY_SOLVER_GRAPH_H

#include "../data/matrix_block_data.h"
#include "../data/matrix_data.h"
#include "../task/decomposition/split_matrix_task.h"
#include "../task/solver/solve_diagonal_task.h"
#include "../task/solver/update_vector_task.h"
#include "../state/solver/solver_state.h"
#include "../state/solver/solver_state_manager.h"
#include <hedgehog/hedgehog.h>

#define CSGraphInNb 3
#define CSGraphIn                        \
  MatrixBlockData<T, VectorBlock>,       \
  MatrixBlockData<T, VectorBlockPhase1>, \
  MatrixBlockData<T, Decomposed>
#define CSGraphOut MatrixBlockData<T, VectorBlockPhase1>, MatrixBlockData<T, Result>

template <typename T, Phases Phase>
class CholeskySolverGraph
        : public hh::Graph<CSGraphInNb, CSGraphIn, CSGraphOut > {
 public:
  CholeskySolverGraph(size_t nbThreadsSolveDiagonal, size_t nbThreadsUpdateVector)
          : hh::Graph<CSGraphInNb, CSGraphIn, CSGraphOut >("Cholesky Solver") {
    auto solveDiagonalTask = std::make_shared<SolveDiagonalTask<T, Phase>>(nbThreadsSolveDiagonal);
    auto updateVectorTask = std::make_shared<UpdateVectorTask<T, Phase>>(nbThreadsUpdateVector);
    auto solverState = std::make_shared<SolverState<T, Phase>>();
    auto solverStateManager = std::make_shared<SolverStateManager<T, Phase>>(solverState);

    this->inputs(solverStateManager);

    this->edges(solverStateManager, solveDiagonalTask);
    this->edges(solveDiagonalTask, solverStateManager);

    this->edges(solverStateManager, updateVectorTask);
    this->edges(updateVectorTask, solverStateManager);

    this->outputs(solverStateManager);
  }
};

#endif
