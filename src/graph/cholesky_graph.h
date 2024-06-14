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
                size_t nbThreadsUpdateTask)
          : hh::Graph<CGraphInNb, CGraphIn, CGraphOut >(
          "Cholesky") {
    auto choleskyDecompositionGraph = std::make_shared<CholeskyDecompositionGraph<T>>(
            nbThreadsComputeDiagonalTask,
            nbThreadsComputeColumnTask,
            nbThreadsUpdateTask);
    auto choleskySolverGraph = std::make_shared<CholeskySolverGraph<T>>();

    this->inputs(choleskyDecompositionGraph);
    this->inputs(choleskySolverGraph);

    this->edges(choleskyDecompositionGraph, choleskySolverGraph);

    this->outputs(choleskySolverGraph);
  }
};

#endif //CHOLESKY_HH_CHOLESKY_GRAPH_H
