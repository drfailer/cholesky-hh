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
