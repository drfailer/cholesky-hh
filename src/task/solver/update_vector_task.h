#ifndef UPDATE_VECTOR_TASK_H
#define UPDATE_VECTOR_TASK_H
#include "../../data/matrix_block_data.h"
#include "../../data/triple_block_data.h"
#include <hedgehog/hedgehog.h>
#include <cblas.h>

template <typename T>
using UpdateVectorTaskInType = TripleBlockData<T, Column, Vector, Vector>;

#define UVTaskInNb 1
#define UVTaskIn UpdateVectorTaskInType<T>
#define UVTaskOut MatrixBlockData<T, Updated>

template <typename T>
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
    size_t k = colBlock->width();
    // todo: the leading dimension should be configurable
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, -1.0, colBlock->get(),
                colBlock->matrixWidth(), solvedVectorBlock->get(), solvedVectorBlock->matrixWidth(), 1.0,
                updatedBlock->get(), updatedBlock->matrixWidth());
    this->addResult(std::make_shared<MatrixBlockData<T, Updated>>(updatedBlock));
  }

  std::shared_ptr<hh::AbstractTask<UVTaskInNb, UVTaskIn, UVTaskOut>>
  copy() override {
    return std::make_shared<UpdateVectorTask<T>>(this->numberThreads());
  }
};

#endif
