#ifndef CHOLESKY_HH_UPDATE_SUBMATRIX_BLOCK_TASK_H
#define CHOLESKY_HH_UPDATE_SUBMATRIX_BLOCK_TASK_H

#include <hedgehog/hedgehog.h>
#include <openblas/cblas.h>
#include "../data/matrix_block_data.h"

template<typename T>
using UpdateSubmatrixBlockInputType =
        std::pair<std::shared_ptr<MatrixBlockData<T, Block>>,
                std::shared_ptr<MatrixBlockData<T, Block>>>;

#define USBTaskInNb 1
#define USBTaskIn UpdateSubmatrixBlockInputType<T>
#define USBTaskOut MatrixBlockData<T, Block>

template<typename T>
class UpdateSubmatrixBlockTask : public hh::AbstractTask<USBTaskInNb, USBTaskIn, USBTaskOut > {
 public:
  UpdateSubmatrixBlockTask(size_t nbThreads) :
          hh::AbstractTask<USBTaskInNb, USBTaskIn, USBTaskOut >("Update Submatrix Block Task", nbThreads) {}

  void execute(std::shared_ptr<UpdateSubmatrixBlockInputType<T>> blocks) override {
    auto ABlock = blocks->first;
    auto LBlock = blocks->second;
    // todo: the leading dimension should be configurable
    cblas_dsyrk(CblasRowMajor, CblasUpper, CblasNoTrans,
                LBlock->blockSize(), LBlock->blockSize(), 1.0, LBlock->get(),
                LBlock->matrixWidth(), 1.0, ABlock->get(), ABlock->matrixWidth());
  }

  std::shared_ptr<hh::AbstractTask<USBTaskInNb, USBTaskIn, USBTaskOut >>
  copy() override {
    return std::make_shared<UpdateSubmatrixBlockTask<T>>(this->numberThreads());
  }
};

#endif //CHOLESKY_HH_UPDATE_SUBMATRIX_BLOCK_TASK_H
