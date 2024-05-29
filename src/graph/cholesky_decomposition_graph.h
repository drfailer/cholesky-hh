#ifndef CHOLESKY_DECOMPOSITION_GRAPH_H
#define CHOLESKY_DECOMPOSITION_GRAPH_H
#include "../data/matrix_block_data.h"
#include "../data/matrix_data.h"
#include "../state/decompose_state.h"
#include "../state/decompose_state_manager.h"
#include "../state/update_submatrix_state.h"
#include "../state/update_submatrix_state_manager.h"
#include "../task/split_matrix_task.h"
#include "../task/compute_diagonal_block_task.h"
#include "../task/compute_column_block_task.h"
#include "../task/update_submatrix_block_task.h"
#include <hedgehog/hedgehog.h>

#define CDGraphInNb 1
#define CDGraphIn MatrixData<T>
#define CDGraphOut MatrixBlockData<T, Result>

template<typename T>
class CholeskyDecompositionGraph
        : public hh::Graph<CDGraphInNb, CDGraphIn, CDGraphOut > {
 public:
  CholeskyDecompositionGraph()
          : hh::Graph<CDGraphInNb, CDGraphIn, CDGraphOut >(
          "Cholesky Decomposition") {
    auto splitMatrixTask = std::make_shared<SplitMatrixTask<T, Block>>();
    auto decomposeState = std::make_shared<DecomposeState<T>>();
    auto decomposeStateManager = std::make_shared<DecomposeStateManager<T>>(decomposeState);
    auto computeDiagonalBlockTask = std::make_shared<ComputeDiagonalBlockTask<T>>(3);
    auto computeColumnBlockTask = std::make_shared<ComputeColumnBlockTask<T>>(3);
    auto updateSubMatrixBlockTask = std::make_shared<UpdateSubMatrixBlockTask<T>>(3);
    auto updateSubMatrixState = std::make_shared<UpdateSubMatrixState<T>>();
    auto updateSubMatrixStateManager = std::make_shared<UpdateSubMatrixStateManager<T>>(
            updateSubMatrixState);

    this->inputs(splitMatrixTask);

    this->edges(splitMatrixTask, decomposeStateManager);
    this->edges(decomposeStateManager, computeDiagonalBlockTask);
    this->edges(computeDiagonalBlockTask, decomposeStateManager);
    this->edges(decomposeStateManager, computeColumnBlockTask);
    this->edges(computeColumnBlockTask, decomposeStateManager);

    this->edges(splitMatrixTask, updateSubMatrixStateManager);
    this->edges(computeColumnBlockTask, updateSubMatrixStateManager);

    this->edges(updateSubMatrixStateManager, updateSubMatrixBlockTask);
    this->edges(updateSubMatrixBlockTask, decomposeStateManager);

    this->outputs(decomposeStateManager);
  }
};

#endif
