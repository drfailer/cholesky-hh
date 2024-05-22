#ifndef CHOLESKY_DECOMPOSITION_GRAPH_H
#define CHOLESKY_DECOMPOSITION_GRAPH_H
#include "../data/matrix_block_data.h"
#include "../data/matrix_data.h"
#include "../state/decompose_state.h"
#include "../state/decompose_state_manager.h"
#include "../task/decompose_block_task.h"
#include "../task/split_matrix_task.h"
#include <hedgehog/hedgehog.h>

#define CDGraphInNb 1
#define CDGraphIn MatrixData<T>
#define CDGraphOut MatrixBlockData<T, Result>

template <typename T>
class CholeskyDecompositionGraph
    : public hh::Graph<CDGraphInNb, CDGraphIn, CDGraphOut> {
public:
  CholeskyDecompositionGraph()
      : hh::Graph<CDGraphInNb, CDGraphIn, CDGraphOut>(
            "Cholesky Decomposition") {
    auto splitMatrixTask = std::make_shared<SplitMatrixTask<T, Block>>();
    auto decomposeBlockTask = std::make_shared<DecomposeBlockTask<T>>(1);
    auto decomposeState = std::make_shared<DecomposeState<T>>();
    auto decomposeStateManager =
        std::make_shared<DecomposeStateManager<T>>(decomposeState);

    this->inputs(splitMatrixTask);
    this->edges(splitMatrixTask, decomposeStateManager);
    this->edges(decomposeStateManager, decomposeBlockTask);
    this->edges(decomposeBlockTask, decomposeStateManager);
    this->outputs(decomposeStateManager);
  }
};

#endif
