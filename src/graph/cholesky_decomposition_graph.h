#ifndef CHOLESKY_DECOMPOSITION_GRAPH_H
#define CHOLESKY_DECOMPOSITION_GRAPH_H
#include "../data/matrix_block_data.h"
#include "../data/matrix_data.h"
#include "../state/decomposition/decompose_state.h"
#include "../state/decomposition/decompose_state_manager.h"
#include "../state/decomposition/update_submatrix_state.h"
#include "../state/decomposition/update_submatrix_state_manager.h"
#include "../task/decomposition/split_matrix_task.h"
#include "../task/decomposition/compute_diagonal_block_task.h"
#include "../task/decomposition/compute_column_block_task.h"
#include "../task/decomposition/update_submatrix_block_task.h"
#include <hedgehog/hedgehog.h>

#define CDGraphInNb 1
#define CDGraphIn MatrixBlockData<T, MatrixBlock>
#define CDGraphOut MatrixBlockData<T, Decomposed>

template<typename T>
class CholeskyDecompositionGraph
        : public hh::Graph<CDGraphInNb, CDGraphIn, CDGraphOut > {
 public:
  CholeskyDecompositionGraph(size_t nbThreadsComputeDiagonalTask,
      size_t nbThreadsComputeColumnTask,
      size_t nbThreadsUpdateTask)
          : hh::Graph<CDGraphInNb, CDGraphIn, CDGraphOut >(
          "Cholesky Decomposition") {
    auto decomposeState = std::make_shared<DecomposeState<T>>();
    auto decomposeStateManager = std::make_shared<DecomposeStateManager<T>>(decomposeState);
    auto computeDiagonalBlockTask = std::make_shared<ComputeDiagonalBlockTask<T>>(nbThreadsComputeDiagonalTask);
    auto computeColumnBlockTask = std::make_shared<ComputeColumnBlockTask<T>>(nbThreadsComputeColumnTask);
    auto updateSubMatrixBlockTask = std::make_shared<UpdateSubMatrixBlockTask<T>>(nbThreadsUpdateTask);
    auto updateSubMatrixState = std::make_shared<UpdateSubMatrixState<T>>();
    auto updateSubMatrixStateManager = std::make_shared<UpdateSubMatrixStateManager<T>>(
            updateSubMatrixState);

    this->inputs(decomposeStateManager);

    this->edges(decomposeStateManager, computeDiagonalBlockTask);
    this->edges(computeDiagonalBlockTask, decomposeStateManager);
    this->edges(decomposeStateManager, computeColumnBlockTask);
    this->edges(computeColumnBlockTask, decomposeStateManager);

    this->edges(decomposeStateManager, updateSubMatrixStateManager);

    this->edges(updateSubMatrixStateManager, updateSubMatrixBlockTask);
    this->edges(updateSubMatrixBlockTask, decomposeStateManager);

    this->outputs(decomposeStateManager);
  }
};

#endif
