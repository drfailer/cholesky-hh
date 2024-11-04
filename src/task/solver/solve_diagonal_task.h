#ifndef SOLVE_DIAGONAL_TASK_H
#define SOLVE_DIAGONAL_TASK_H
#include "../../data/matrix_block_data.h"
#include "../../data/solver/phases.h"
#include <hedgehog/hedgehog.h>
#include <cblas.h>

template <typename T>
using SolveDiagonalTaskInType =
  std::pair<
    std::shared_ptr<MatrixBlockData<T, Diagonal>>,
    std::shared_ptr<MatrixBlockData<T, Vector>>>;

#define SDTaskInNb 1
#define SDTaskIn SolveDiagonalTaskInType<T>
#define SDTaskOut MatrixBlockData<T, Vector>

template <typename T, Phases Phase>
class SolveDiagonalTask : public hh::AbstractTask<SDTaskInNb, SDTaskIn, SDTaskOut> {
 public:
  SolveDiagonalTask(size_t nbThreads)
    : hh::AbstractTask<SDTaskInNb, SDTaskIn, SDTaskOut>("Solve Diagonal Task", nbThreads) {}

  void execute(std::shared_ptr<SolveDiagonalTaskInType<T>> blocks) override {
    auto diagBlock = blocks->first;
    auto vecBlock = blocks->second;
    // todo: leading dimension should be configurable
    if constexpr (Phase == Phases::First) {
      cblas_dtrsm(CblasRowMajor, CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit,
                  vecBlock->height(), vecBlock->width(), 1.0, diagBlock->get(),
                  diagBlock->matrixWidth(), vecBlock->get(), vecBlock->matrixWidth());
    } else {
      cblas_dtrsm(CblasRowMajor, CblasLeft, CblasLower, CblasTrans, CblasNonUnit,
                  vecBlock->height(), vecBlock->width(), 1.0, diagBlock->get(),
                  diagBlock->matrixWidth(), vecBlock->get(), vecBlock->matrixWidth());
    }
    this->addResult(vecBlock);
  }

  std::shared_ptr<hh::AbstractTask<SDTaskInNb, SDTaskIn, SDTaskOut>>
  copy() override {
    return std::make_shared<SolveDiagonalTask<T, Phase>>(this->numberThreads());
  }
};

#endif
