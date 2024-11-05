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

#ifndef SPLIT_MATRIX_TASK_H
#define SPLIT_MATRIX_TASK_H

#include "hedgehog/hedgehog/hedgehog.h"
#include <memory>
#include "../../data/matrix_block_data.h"
#include "../../data/matrix_data.h"

#define SMTaskInNb 2
#define SMTaskIn MatrixData<T, MatrixTypes::Matrix>, MatrixData<T, MatrixTypes::Vector>
#define SMTaskOut MatrixBlockData<T, MatrixBlock>, MatrixBlockData<T, VectorBlock>

template <typename T>
class SplitMatrixTask
        : public hh::AbstractAtomicTask<SMTaskInNb, SMTaskIn, SMTaskOut > {
 public:
  SplitMatrixTask()
          : hh::AbstractAtomicTask<SMTaskInNb, SMTaskIn, SMTaskOut >("Split matrix task") {
  }

  void execute(std::shared_ptr<MatrixData<T, MatrixTypes::Matrix>> matrix) override {
    for (size_t iBlock = 0; iBlock < matrix->nbBlocksRows(); ++iBlock) {
      for (size_t jBlock = 0; jBlock <= iBlock; ++jBlock) {
        this->addResult(std::make_shared<MatrixBlockData<T, MatrixBlock>>(
                std::min(matrix->blockSize(), matrix->width() - (jBlock * matrix->blockSize())),
                std::min(matrix->blockSize(), matrix->height() - (iBlock * matrix->blockSize())),
                matrix->nbBlocksRows(), matrix->nbBlocksCols(),
                jBlock, iBlock, matrix->width(), matrix->height(),
                matrix->get() + iBlock * matrix->blockSize() * matrix->width() +
                jBlock * matrix->blockSize(), matrix->get()));
      }
    }
  }

  void execute(std::shared_ptr<MatrixData<T, MatrixTypes::Vector>> vector) override {
    for (size_t iBlock = 0; iBlock < vector->nbBlocksRows(); ++iBlock) {
      this->addResult(std::make_shared<MatrixBlockData<T, VectorBlock>>(
              1,
              std::min(vector->blockSize(), vector->height() - (iBlock * vector->blockSize())),
              vector->nbBlocksRows(), vector->nbBlocksCols(), 0, iBlock, vector->width(),
              vector->height(), vector->get() + iBlock * vector->blockSize() * vector->width(),
              vector->get()));
    }
  }
};

#endif
