// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the
// software in any medium, provided that you keep intact this entire notice. You may improve, modify and create
// derivative works of the software or any portion of the software, and you may copy and distribute such modifications
// or works. Modified works should carry a notice stating that you changed the software and should note the date and
// nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the
// source of the software. NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND,
// EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
// WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE
// CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS
// THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE. You
// are solely responsible for determining the appropriateness of using and distributing the software and you assume
// all risks associated with its use, including but not limited to the risks and costs of program errors, compliance
// with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of
// operation. This software is not intended to be used in any situation where a failure could cause risk of injury or
// damage to property. The software developed by NIST employees is not subject to copyright protection within the
// United States.

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
