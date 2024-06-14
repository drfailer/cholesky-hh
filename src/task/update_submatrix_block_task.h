#ifndef CHOLESKY_HH_UPDATE_SUBMATRIX_BLOCK_TASK_H
#define CHOLESKY_HH_UPDATE_SUBMATRIX_BLOCK_TASK_H

#include <hedgehog/hedgehog.h>
#include <cblas.h>
#include "../data/matrix_block_data.h"
#include "../data/triple_block_data.h"

template<typename T>
using UpdateSubmatrixBlockInputType = TripleBlockData<T>;

#define USBTaskInNb 1
#define USBTaskIn UpdateSubmatrixBlockInputType<T>
#define USBTaskOut MatrixBlockData<T, Updated>

template<typename T>
class UpdateSubMatrixBlockTask : public hh::AbstractAtomicTask<USBTaskInNb, USBTaskIn, USBTaskOut > {
 public:
  explicit UpdateSubMatrixBlockTask(size_t nbThreads) :
          hh::AbstractAtomicTask<USBTaskInNb, USBTaskIn, USBTaskOut >("Update Submatrix Block Task",
                                                                nbThreads) {}

  /// @brief Receives 3 blocks. The first two blocks are on the column that is processed. The third
  /// block will be updated. Here we do $updatedB = updatedB - colB1.colB2^T$.
  void execute(std::shared_ptr<UpdateSubmatrixBlockInputType<T>> blocks) override {
    auto colBlock1 = blocks->first;
    auto colBlock2 = blocks->second;
    auto updatedBlock = blocks->third;
    size_t m = updatedBlock->height();
    size_t n = updatedBlock->width();
    size_t k = colBlock1->width();
    // todo: the leading dimension should be configurable
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, m, n, k, -1.0, colBlock1->get(),
                colBlock1->matrixWidth(), colBlock2->get(), colBlock2->matrixWidth(), 1.0,
                updatedBlock->get(), updatedBlock->matrixWidth());
    this->addResult(std::make_shared<MatrixBlockData<T, Updated>>(std::move(updatedBlock)));
  }

  std::shared_ptr<hh::AbstractTask<USBTaskInNb, USBTaskIn, USBTaskOut >>
  copy() override {
    return std::make_shared<UpdateSubMatrixBlockTask<T>>(this->numberThreads());
  }
};

#endif //CHOLESKY_HH_UPDATE_SUBMATRIX_BLOCK_TASK_H
