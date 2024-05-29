#ifndef CHOLESKY_HH_COMPUTE_COLUMN_BLOCK_TASK_H
#define CHOLESKY_HH_COMPUTE_COLUMN_BLOCK_TASK_H

#include <hedgehog/hedgehog.h>
#include <cblas.h>
#include "../data/matrix_block_data.h"

template<typename T>
using CCBTaskInputType =
        std::pair<std::shared_ptr<MatrixBlockData<T, Diagonal>>,
                std::shared_ptr<MatrixBlockData<T, Block>>>;

#define CCBTaskInNb 1
#define CCBTaskIn CCBTaskInputType<T>
#define CCBTaskOut MatrixBlockData<T, Result>

template<typename T>
class ComputeColumnBlockTask : public hh::AbstractTask<CCBTaskInNb, CCBTaskIn, CCBTaskOut > {
 public:
  explicit ComputeColumnBlockTask(size_t nbThreads) : hh::AbstractTask<CCBTaskInNb, CCBTaskIn, CCBTaskOut >(
          "Compute Column Block Task", nbThreads) {}

  void execute(std::shared_ptr<CCBTaskInputType<T>> blocks) override {
    auto diagBlock = blocks->first;
    auto colBlock = blocks->second;
    // todo: leading dimension should be configurable
    cblas_dtrsm(CblasRowMajor, CblasLeft, CblasLower,
                CblasNoTrans, CblasNonUnit, diagBlock->blockSize(), diagBlock->blockSize(), 1.0,
                diagBlock->get(), diagBlock->matrixWidth(), colBlock->get(),
                colBlock->matrixWidth());
  }

  std::shared_ptr<hh::AbstractTask<CCBTaskInNb, CCBTaskIn, CCBTaskOut>> copy() override {
    return std::make_shared<ComputeColumnBlockTask<T>>(this->numberThreads());
  }
};

#endif //CHOLESKY_HH_COMPUTE_COLUMN_BLOCK_TASK_H