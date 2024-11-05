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

#ifndef CHOLESKY_HH_COMPUTE_DIAGONAL_BLOCK_TASK_H
#define CHOLESKY_HH_COMPUTE_DIAGONAL_BLOCK_TASK_H

#include "hedgehog/hedgehog/hedgehog.h"
#include <lapack.h>
#include "../../data/matrix_block_data.h"

#define CDBTaskInNb 1
#define CDBTaskIn MatrixBlockData<T, Diagonal>
#define CDBTaskOut MatrixBlockData<T, Diagonal>

template<typename T>
class ComputeDiagonalBlockTask : public hh::AbstractAtomicTask<CDBTaskInNb, CDBTaskIn, CDBTaskOut > {
 public:
  explicit ComputeDiagonalBlockTask(size_t nbThreads) :
          hh::AbstractAtomicTask<CDBTaskInNb, CDBTaskIn, CDBTaskOut >("Compute Diagonal Block Task", nbThreads) {}

  void execute(std::shared_ptr<MatrixBlockData<T, Diagonal>> block) override {
    int32_t n = block->height();
    // todo: leading dimension should be configurable
    int32_t lda = block->matrixWidth();
    int32_t info = 0;
    /* LAPACK_dpotf2((char*) "U", &n, block->get(), &lda, &info); */
    LAPACK_dpotrf((char*) "U", &n, block->get(), &lda, &info);
    this->addResult(block);
  }

  std::shared_ptr<hh::AbstractTask<CDBTaskInNb, CDBTaskIn, CDBTaskOut>> copy() override {
    return std::make_shared<ComputeDiagonalBlockTask<T>>(this->numberThreads());
  }
};

#endif //CHOLESKY_HH_COMPUTE_DIAGONAL_BLOCK_TASK_H
