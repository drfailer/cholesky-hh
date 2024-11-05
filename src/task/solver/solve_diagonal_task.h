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
