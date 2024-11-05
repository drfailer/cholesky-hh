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

#ifndef SOLVER_STATE_H
#define SOLVER_STATE_H

#include "../../data/matrix_block_data.h"
#include "../../data/solver/phases.h"
#include "../../task/solver/update_vector_task.h"
#include "../../task/solver/solve_diagonal_task.h"
#include <vector>
#include <list>

#define SStateInNb 5
#define SStateIn                         \
  MatrixBlockData<T, Decomposed>,        \
  MatrixBlockData<T, VectorBlock>,       \
  MatrixBlockData<T, VectorBlockPhase1>, \
  MatrixBlockData<T, Vector>,            \
  MatrixBlockData<T, Updated>
#define SStateOut                         \
  SolveDiagonalTaskInType<T>,             \
  UpdateVectorTaskInType<T>,              \
  MatrixBlockData<T, VectorBlockPhase1>,  \
  MatrixBlockData<T, Result>

template <typename T, Phases Phase>
class SolverState : public hh::AbstractState<SStateInNb, SStateIn, SStateOut > {
 public:
  SolverState() : hh::AbstractState<SStateInNb, SStateIn, SStateOut >() {}

  /* Decomposed ***************************************************************/

  /// @brief Receives the blocks from the cholesky decomposition graph
  void execute(std::shared_ptr<MatrixBlockData<T, Decomposed>> block) override {
    if (blocks_.size() == 0) {
      nbBlocksCols_ = block->nbBlocksCols();
      nbBlocksRows_ = block->nbBlocksRows();
      blocks_ = std::vector<std::shared_ptr<MatrixBlockData<T, MatrixBlock>>>(
              block->nbBlocksCols() * block->nbBlocksRows(), nullptr);
    }

    blocks_[block->idx()] = std::make_shared<MatrixBlockData<T, MatrixBlock>>(block);
    sendSolveDiagPending();
    sendUpdatePending();
  }

  /* VectorBlock **************************************************************/

  /// @brief Receives the vector blocks from the split matrix task
  void execute(std::shared_ptr<MatrixBlockData<T, VectorBlock>> vecBlock) override {
    if constexpr (Phase == Phases::First) {
      if (vectorBlocks_.size() == 0) {
        vectorBlocks_ = std::vector<std::shared_ptr<MatrixBlockData<T, Vector>>>(
                vecBlock->nbBlocksRows(), nullptr);
      }

      vectorBlocks_[vecBlock->idx()] = std::make_shared<MatrixBlockData<T, Vector>>(vecBlock);

      if (vecBlock->rank() == vecBlock->y()) {
        solveDiagPending_.emplace_back(SolveDiagonalIdx(
                vecBlock->y() * nbBlocksCols_ + vecBlock->y(),
                vecBlock->idx()
        ));
      }
      sendSolveDiagPending();
      sendUpdatePending();
    }
  }

  /* VectorBlockPhase1 ********************************************************/

  /// @brief Receives the vector blocks from the phase 1
  void execute(std::shared_ptr<MatrixBlockData<T, VectorBlockPhase1>> vecBlock) override {
    if constexpr (Phase == Phases::Second) {
      if (vectorBlocks_.size() == 0) {
        vectorBlocks_ = std::vector<std::shared_ptr<MatrixBlockData<T, Vector>>>(
                vecBlock->nbBlocksRows(), nullptr);
      }

      vecBlock->rank(nbBlocksRows_ - vecBlock->y());
      vectorBlocks_[vecBlock->idx()] = std::make_shared<MatrixBlockData<T, Vector>>(vecBlock);

      if (vecBlock->rank() == 1) {
        solveDiagPending_.emplace_back(SolveDiagonalIdx(
                vecBlock->y() * nbBlocksCols_ + vecBlock->y(),
                vecBlock->idx()
        ));
      }
      sendSolveDiagPending();
      sendUpdatePending();
    }
  }

  /* Vector *******************************************************************/

  /// @brief Receives vector blocks from the solve diagonal task (these are results of the graph)
  void execute(std::shared_ptr<MatrixBlockData<T, Vector>> block) override {
    if constexpr (Phase == Phases::First) {
      vectorBlocks_[block->idx()]->incRank();

      // update all the blocks beneath
      for (size_t i = block->y() + 1; i < nbBlocksRows_; ++i) {
        size_t colBlockIdx = i * nbBlocksCols_ + block->y();
        updateVecPending_.emplace_back(UpdateVectorIdx(colBlockIdx, block->idx(), i));
      }
      this->addResult(std::make_shared<MatrixBlockData<T, VectorBlockPhase1>>(block));
    } else {
      vectorBlocks_[block->idx()]->decRank();

      // update all the blocks above
      for (size_t i = 0; i < block->y(); ++i) {
        size_t colBlockIdx =
                block->y() * nbBlocksCols_ + i; // we invert because the matrix should be translated
        updateVecPending_.emplace_back(UpdateVectorIdx(colBlockIdx, block->idx(), i));
      }
      this->addResult(std::make_shared<MatrixBlockData<T, Result>>(block));
    }
    sendUpdatePending();
  }

  /* Updated ******************************************************************/

  /// @brief Receives vector blocs from the update vector task.
  void execute(std::shared_ptr<MatrixBlockData<T, Updated>> block) override {
    if constexpr (Phase == Phases::First) {
      size_t rank = vectorBlocks_[block->idx()]->incRank();

      if (rank == block->y()) {
        solveDiagPending_.emplace_back(SolveDiagonalIdx(
                block->y() * nbBlocksCols_ + block->y(),
                block->idx()
        ));
      }
    } else {
      size_t rank = vectorBlocks_[block->idx()]->decRank();

      if (rank == 1) {
        solveDiagPending_.emplace_back(SolveDiagonalIdx(
                block->y() * nbBlocksCols_ + block->y(),
                block->idx()
        ));
      }
    }

    sendSolveDiagPending();
    sendUpdatePending();
  }

  /* idDone ******************************************************************/

  [[nodiscard]] bool isDone() const {
    if constexpr (Phase == Phases::First) {
      return !vectorBlocks_.empty() && vectorBlocks_.back() &&
             vectorBlocks_.back()->rank() > vectorBlocks_.back()->y();
    } else {
      return !vectorBlocks_.empty() && vectorBlocks_.front() && vectorBlocks_.front()->rank() == 0;
    }
  }

 private:

  /* Types *******************************************************************/

  struct SolveDiagonalIdx {
    SolveDiagonalIdx(size_t diag, size_t vec) :
            diag(diag), vec(vec) {}
    size_t diag; // diagonal element
    size_t vec;  // vector element
  };

  struct UpdateVectorIdx {
    UpdateVectorIdx(size_t col, size_t solvedVec, size_t updatedVec) :
            col(col), solvedVec(solvedVec), updatedVec(updatedVec) {}
    size_t col;        // column element
    size_t solvedVec;  // solved vector part
    size_t updatedVec; // the part of the vector that we want to update
  };

  /* Variables ***************************************************************/

  std::vector<std::shared_ptr<MatrixBlockData<T, MatrixBlock>>> blocks_ = {};
  std::vector<std::shared_ptr<MatrixBlockData<T, Vector>>> vectorBlocks_ = {};
  std::list<SolveDiagonalIdx> solveDiagPending_ = {};
  std::list<UpdateVectorIdx> updateVecPending_ = {};
  size_t nbBlocksCols_ = 0;
  size_t nbBlocksRows_ = 0;

  /* Send functions **********************************************************/

  /// @brief Send all pending triplet to update if all the blocks are ready
  void sendUpdatePending() {
    auto it = updateVecPending_.begin();

    while (it != updateVecPending_.end()) {
      auto col = blocks_[it->col];
      auto solvedVec = vectorBlocks_[it->solvedVec];
      auto updatedVec = vectorBlocks_[it->updatedVec];
      bool isReady;

      if constexpr (Phase == Phases::First) {
        isReady = updatedVec && updatedVec->rank() == (solvedVec->rank() - 1);
      } else {
        isReady = updatedVec && updatedVec->rank() == (solvedVec->y() - updatedVec->y() + 1);
      }

      if (col && isReady) {
        this->addResult(std::make_shared<UpdateVectorTaskInType<T>>(
                std::make_shared<MatrixBlockData<T, Column>>(col),
                solvedVec,
                updatedVec));
        it = updateVecPending_.erase(it);
      } else {
        it++;
      }
    }
  }


  /// @brief Send all the pending diagonal blocks to update if they are ready
  void sendSolveDiagPending() {
    if (blocks_.empty()) {
      return;
    }

    auto it = solveDiagPending_.begin();

    while (it != solveDiagPending_.end()) {
      auto diag = blocks_[it->diag];
      auto vec = vectorBlocks_[it->vec];

      if (diag && vec) {
        this->addResult(std::make_shared<SolveDiagonalTaskInType<T>>(
                std::make_shared<MatrixBlockData<T, Diagonal>>(diag),
                vec));
        it = solveDiagPending_.erase(it);
      } else {
        it++;
      }
    }
  }
};

#endif
