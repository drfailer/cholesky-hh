#ifndef CHOLESKY_HH_CHOLESKY_GRAPH_H
#define CHOLESKY_HH_CHOLESKY_GRAPH_H

#include <hedgehog/hedgehog.h>
#include "../data/matrix_data.h"
#include "../data/matrix_block_data.h"
#include "cholesky_decomposition_graph.h"
#include "cholesky_solver_graph.h"

#define CGraphInNb 2
#define CGraphIn MatrixData<T, MatrixTypes::Matrix>, MatrixData<T, MatrixTypes::Vector>
#define CGraphOut MatrixBlockData<T, Result>

template <typename T>
class CholeskyGraph
        : public hh::Graph<CGraphInNb, CGraphIn, CGraphOut > {
 public:
  CholeskyGraph(size_t nbThreadsComputeDiagonalTask,
                size_t nbThreadsComputeColumnTask,
                size_t nbThreadsUpdateTask,
                size_t nbThreadsSolveDiagonal,
                size_t nbThreadsUpdateVector)
          : hh::Graph<CGraphInNb, CGraphIn, CGraphOut >("Cholesky") {
    auto splitTask = std::make_shared<SplitMatrixTask<T>>();
    auto choleskyDecompositionGraph = std::make_shared<CholeskyDecompositionGraph<T>>(
            nbThreadsComputeDiagonalTask,
            nbThreadsComputeColumnTask,
            nbThreadsUpdateTask);
    auto choleskySolverGraph1 =
            std::make_shared<CholeskySolverGraph<T, Phases::First>>(
                    nbThreadsSolveDiagonal, nbThreadsUpdateVector);
    auto choleskySolverGraph2 =
            std::make_shared<CholeskySolverGraph<T, Phases::Second>>(
                    nbThreadsSolveDiagonal, nbThreadsUpdateVector);

    this->inputs(splitTask);

    this->edges(splitTask, choleskyDecompositionGraph);
    this->edges(splitTask, choleskySolverGraph1);

    this->edges(choleskySolverGraph1, choleskySolverGraph2);
    this->edges(choleskyDecompositionGraph, choleskySolverGraph1);
    this->edges(choleskyDecompositionGraph, choleskySolverGraph2);

    this->outputs(choleskySolverGraph2);
  }
};

#endif //CHOLESKY_HH_CHOLESKY_GRAPH_H
