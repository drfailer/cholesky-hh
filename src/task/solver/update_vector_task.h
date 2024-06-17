#ifndef UPDATE_VECTOR_TASK_H
#define UPDATE_VECTOR_TASK_H
#include "../../data/matrix_block_data.h"
#include "../../data/triple_block_data.h"
#include "../../data/solver/phases.h"
#include <hedgehog/hedgehog.h>
#include <cblas.h>

template <typename T>
using UpdateVectorTaskInType = TripleBlockData<T, Column, Vector, Vector>;

#define UVTaskInNb 1
#define UVTaskIn UpdateVectorTaskInType<T>
#define UVTaskOut MatrixBlockData<T, Updated>

/// @brief Update the result vector by substracting the matrix times a solved vector block (removes
/// known variables from the equation)
template <typename T, Phases Phase>
class UpdateVectorTask : public hh::AbstractTask<UVTaskInNb, UVTaskIn, UVTaskOut> {
 public:
  UpdateVectorTask(size_t nbThreads)
    : hh::AbstractTask<UVTaskInNb, UVTaskIn, UVTaskOut>("Update Vector task", nbThreads) {}

  /// @brief updatedBlock -= colBlock.solvedVectorBlock
  void execute(std::shared_ptr<UpdateVectorTaskInType<T>> blocks) override {
    auto colBlock = blocks->first;           // block in the triangular matrix
    auto solvedVectorBlock = blocks->second; // already solved vector block
    auto updatedBlock = blocks->third;       // vector block that we want to update
    size_t m = updatedBlock->height();
    size_t n = updatedBlock->width();
    size_t k = solvedVectorBlock->height();
    // todo: the leading dimension should be configurable
    if constexpr (Phase == Phases::First) {
      cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, -1.0, colBlock->get(),
                  colBlock->matrixWidth(), solvedVectorBlock->get(), solvedVectorBlock->matrixWidth(), 1.0,
                  updatedBlock->get(), updatedBlock->matrixWidth());
    } else {
      cblas_dgemm(CblasRowMajor, CblasTrans, CblasNoTrans, m, n, k, -1.0, colBlock->get(),
                  colBlock->matrixWidth(), solvedVectorBlock->get(), solvedVectorBlock->matrixWidth(), 1.0,
                  updatedBlock->get(), updatedBlock->matrixWidth());
    }
    this->addResult(std::make_shared<MatrixBlockData<T, Updated>>(updatedBlock));
  }

  std::shared_ptr<hh::AbstractTask<UVTaskInNb, UVTaskIn, UVTaskOut>>
  copy() override {
    return std::make_shared<UpdateVectorTask<T, Phase>>(this->numberThreads());
  }
};

#endif
