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

#ifndef CHOLESKY_HH_COMPUTE_COLUMN_BLOCK_TASK_H
#define CHOLESKY_HH_COMPUTE_COLUMN_BLOCK_TASK_H

#include "hedgehog/hedgehog/hedgehog.h"
#include <cblas.h>
#include "../../data/matrix_block_data.h"

template<typename T>
using CCBTaskInputType =
        std::pair<std::shared_ptr<MatrixBlockData<T, Diagonal>>,
                std::shared_ptr<MatrixBlockData<T, MatrixBlock>>>;

#define CCBTaskInNb 1
#define CCBTaskIn CCBTaskInputType<T>
#define CCBTaskOut MatrixBlockData<T, Column>

template<typename T>
class ComputeColumnBlockTask : public hh::AbstractAtomicTask<CCBTaskInNb, CCBTaskIn, CCBTaskOut > {
 public:
  explicit ComputeColumnBlockTask(size_t nbThreads)
          : hh::AbstractAtomicTask<CCBTaskInNb, CCBTaskIn, CCBTaskOut >(
          "Compute Column Block Task", nbThreads) {}

  /// @brief Receives a pair of blocks. The first block is a the diagonal element on the column and
  /// the second block is the one that will be updated $(colB = colB(diagB^T)^{-1})$.
  void execute(std::shared_ptr<CCBTaskInputType<T>> blocks) override {
    auto diagBlock = blocks->first;
    auto colBlock = blocks->second;
    // todo: leading dimension should be configurable
    cblas_dtrsm(CblasRowMajor, CblasRight, CblasLower, CblasTrans, CblasNonUnit,
                colBlock->height(), colBlock->width(), 1.0, diagBlock->get(),
                diagBlock->matrixWidth(), colBlock->get(), colBlock->matrixWidth());
    this->addResult(std::make_shared<MatrixBlockData<T, Column>>(std::move(colBlock)));
  }

  std::shared_ptr<hh::AbstractTask<CCBTaskInNb, CCBTaskIn, CCBTaskOut>> copy() override {
    return std::make_shared<ComputeColumnBlockTask<T>>(this->numberThreads());
  }
};

#endif //CHOLESKY_HH_COMPUTE_COLUMN_BLOCK_TASK_H
