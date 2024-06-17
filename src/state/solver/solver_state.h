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
    sendSolveDiagPendings();
    sendUpdatePendings();
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
        solveDiagPendings_.emplace_back(SolveDiagonalIdx(
                vecBlock->y() * nbBlocksCols_ + vecBlock->y(),
                vecBlock->idx()
        ));
      }
      sendSolveDiagPendings();
      sendUpdatePendings();
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
        solveDiagPendings_.emplace_back(SolveDiagonalIdx(
                vecBlock->y() * nbBlocksCols_ + vecBlock->y(),
                vecBlock->idx()
        ));
      }
      sendSolveDiagPendings();
      sendUpdatePendings();
    }
  }

  /* Vector *******************************************************************/

  /// @brief Receives vector blocks from the solve diagonal task (these are results of the graph)
  void execute(std::shared_ptr<MatrixBlockData<T, Vector>> block) override {
    if constexpr (Phase == Phases::First) {
     vectorBlocks_[block->idx()]->incRank();

      for (size_t i = block->y() + 1; i < nbBlocksRows_; ++i) {
        size_t colBlockIdx = i * nbBlocksCols_ + block->y();
        updateVecPendings_.emplace_back(UpdateVectorIdx(
                colBlockIdx,
                block->idx(),
                i
        ));
      }

      this->addResult(std::make_shared<MatrixBlockData<T, VectorBlockPhase1>>(block));
    } else {
      vectorBlocks_[block->idx()]->decRank();

      for (size_t i = 0; i < block->y(); ++i) {
        size_t colBlockIdx = block->y() * nbBlocksCols_ + i; // we invert because the matrix should be translated
        updateVecPendings_.emplace_back(UpdateVectorIdx(
                colBlockIdx,
                block->idx(),
                i
        ));
      }

     this->addResult(std::make_shared<MatrixBlockData<T, Result>>(block));
    }
    sendUpdatePendings();
  }

  /* Updated ******************************************************************/

  /// @brief Receives vector blocs from the update vector task.
  void execute(std::shared_ptr<MatrixBlockData<T, Updated>> block) override {
    if constexpr (Phase == Phases::First) {
      vectorBlocks_[block->idx()]->incRank();
      block->incRank();

      if (block->rank() == block->y()) {
        solveDiagPendings_.emplace_back(SolveDiagonalIdx(
                block->y() * nbBlocksCols_ + block->y(),
                block->idx()
        ));
      }
    } else {
      vectorBlocks_[block->idx()]->decRank();
      block->decRank();

      if (block->rank() == 1) {
        solveDiagPendings_.emplace_back(SolveDiagonalIdx(
                block->y() * nbBlocksCols_ + block->y(),
                block->idx()
        ));
      }
    }

    sendSolveDiagPendings();
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

  std::vector<std::shared_ptr<MatrixBlockData<T, MatrixBlock>>> blocks_ = {};
  std::vector<std::shared_ptr<MatrixBlockData<T, Vector>>> vectorBlocks_ = {};
  std::list<SolveDiagonalIdx> solveDiagPendings_ = {};
  std::list<UpdateVectorIdx> updateVecPendings_ = {};
  size_t nbBlocksCols_ = 0;
  size_t nbBlocksRows_ = 0;

  void sendUpdatePendings() {
    auto it = updateVecPendings_.begin();

    while (it != updateVecPendings_.end()) {
      auto col = blocks_[it->col];
      auto solvedVec = vectorBlocks_[it->solvedVec];
      auto updatedVec = vectorBlocks_[it->updatedVec];

      if (col && updatedVec) {
        this->addResult(std::make_shared<UpdateVectorTaskInType<T>>(
                std::make_shared<MatrixBlockData<T, Column>>(col),
                solvedVec,
                updatedVec));
        it = updateVecPendings_.erase(it);
      } else {
        it++;
      }
    }
  }


  void sendSolveDiagPendings() {
    if (blocks_.empty()) {
      return;
    }

    auto it = solveDiagPendings_.begin();

    while (it != solveDiagPendings_.end()) {
      auto diag = blocks_[it->diag];
      auto vec = vectorBlocks_[it->vec];

      if (diag && vec) {
        this->addResult(std::make_shared<SolveDiagonalTaskInType<T>>(
                std::make_shared<MatrixBlockData<T, Diagonal>>(diag),
                vec));
        it = solveDiagPendings_.erase(it);
      } else {
        it++;
      }
    }
  }
};

#endif
