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

#ifndef CHOLESKY_HH_TRIPLE_BLOCK_DATA_H
#define CHOLESKY_HH_TRIPLE_BLOCK_DATA_H

#include "./matrix_block_data.h"

/// @brief Container that is used to send data to the update task.
template <typename T, BlockTypes B1 = MatrixBlock, BlockTypes B2 = MatrixBlock, BlockTypes B3 = MatrixBlock>
struct TripleBlockData {
  TripleBlockData(std::shared_ptr<MatrixBlockData<T, B1>> first,
                  std::shared_ptr<MatrixBlockData<T, B2>> second,
                  std::shared_ptr<MatrixBlockData<T, B3>> third) :
          first(first), second(second), third(third) {}

  std::shared_ptr<MatrixBlockData<T, B1>> first = nullptr;
  std::shared_ptr<MatrixBlockData<T, B2>> second = nullptr;
  std::shared_ptr<MatrixBlockData<T, B3>> third = nullptr;
};

#endif //CHOLESKY_HH_TRIPLE_BLOCK_DATA_H
