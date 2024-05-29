#ifndef CHOLESKY_HH_COMPUTE_DIAGONAL_BLOCK_TASK_H
#define CHOLESKY_HH_COMPUTE_DIAGONAL_BLOCK_TASK_H

#include <hedgehog/hedgehog.h>
#include <cblas.h>
#include <openblas/lapack.h>
#include "../data/matrix_block_data.h"

#define CDBTaskInNb 1
#define CDBTaskIn MatrixBlockData<T, Diagonal>
#define CDBTaskOut MatrixBlockData<T, Result>

template<typename T>
class ComputeDiagonalBlockTask : public hh::AbstractTask<CDBTaskInNb, CDBTaskIn, CDBTaskOut > {
 public:
  explicit ComputeDiagonalBlockTask(size_t nbThreads) :
          hh::AbstractTask<CDBTaskInNb, CDBTaskIn, CDBTaskOut >("Compute Diagonal Block Task",
                                                                      nbThreads) {}

  void execute(std::shared_ptr<MatrixBlockData<T, Diagonal>> block) override {
    int32_t n = block->blockSize();
    // todo: leading dimension should be configurable
    int32_t lda = block->matrixWidth();
    int32_t info = 0;
    LAPACK_dpotf2("U", &n, block->get(), &lda, &info);
    this->addResult(std::make_shared<MatrixBlockData<T, Result>>(block));
  }

  std::shared_ptr<hh::AbstractTask<CDBTaskInNb, CDBTaskIn, CDBTaskOut>> copy() override {
    return std::make_shared<ComputeDiagonalBlockTask<T>>(this->numberThreads());
  }
};

#endif //CHOLESKY_HH_COMPUTE_DIAGONAL_BLOCK_TASK_H
