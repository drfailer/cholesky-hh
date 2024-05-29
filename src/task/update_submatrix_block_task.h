#ifndef CHOLESKY_HH_UPDATE_SUBMATRIX_BLOCK_TASK_H
#define CHOLESKY_HH_UPDATE_SUBMATRIX_BLOCK_TASK_H

#include <hedgehog/hedgehog.h>
#include <openblas/cblas.h>
#include "../data/matrix_block_data.h"
#include "../data/triple_block_data.h"

template<typename T>
using UpdateSubmatrixBlockInputType = TripleBlockData<T>;

#define USBTaskInNb 1
#define USBTaskIn UpdateSubmatrixBlockInputType<T>
#define USBTaskOut MatrixBlockData<T, Updated>

template<typename T>
class UpdateSubMatrixBlockTask : public hh::AbstractTask<USBTaskInNb, USBTaskIn, USBTaskOut > {
 public:
  explicit UpdateSubMatrixBlockTask(size_t nbThreads) :
          hh::AbstractTask<USBTaskInNb, USBTaskIn, USBTaskOut >("Update Submatrix Block Task",
                                                                nbThreads) {}

  void execute(std::shared_ptr<UpdateSubmatrixBlockInputType<T>> blocks) override {
    auto L1Block = blocks->first;
    auto L2Block = blocks->second;
    auto ABlock = blocks->third;
    // todo: the leading dimension should be configurable
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, L1Block->blockSize(),
                L1Block->blockSize(), ABlock->blockSize(), -1.0, L1Block->get(),
                L1Block->matrixWidth(), L2Block->get(),
                L2Block->matrixWidth(), 1.0, ABlock->get(), ABlock->matrixWidth());
    this->addResult(std::make_shared<MatrixBlockData<T, Updated>>(ABlock));
  }

  std::shared_ptr<hh::AbstractTask<USBTaskInNb, USBTaskIn, USBTaskOut >>
  copy() override {
    return std::make_shared<UpdateSubMatrixBlockTask<T>>(this->numberThreads());
  }
};

#endif //CHOLESKY_HH_UPDATE_SUBMATRIX_BLOCK_TASK_H
