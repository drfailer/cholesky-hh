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

#ifndef CHOLESKY_HH_UPDATE_SUBMATRIX_BLOCK_TASK_H
#define CHOLESKY_HH_UPDATE_SUBMATRIX_BLOCK_TASK_H

#include "hedgehog/hedgehog/hedgehog.h"
#include <cblas.h>
#include "../../data/matrix_block_data.h"
#include "../../data/triple_block_data.h"

template <typename T>
using UpdateSubmatrixBlockInputType = TripleBlockData<T>;

#define USBTaskInNb 1
#define USBTaskIn UpdateSubmatrixBlockInputType<T>
#define USBTaskOut MatrixBlockData<T, Updated>

template <typename T>
class UpdateSubMatrixBlockTask
        : public hh::AbstractAtomicTask<USBTaskInNb, USBTaskIn, USBTaskOut > {
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
