#ifndef SOLVER_STATE_H
#define SOLVER_STATE_H
#include "../../data/matrix_block_data.h"
#include "../../task/solver/update_vector_task.h"
#include "../../task/solver/solve_diagonal_task.h"
#include <vector>
#include <list>

#define SStateInNb 4
#define SStateIn \
  MatrixBlockData<T, Result>, MatrixBlockData<T, Block>, \
  MatrixBlockData<T, Vector>, MatrixBlockData<T, Updated>
#define SStateOut \
  SolveDiagonalTaskInType<T>, UpdateVectorTaskInType<T>, \
  MatrixBlockData<T, Result>

template <typename T>
class SolverState : public hh::AbstractState<SStateInNb, SStateIn, SStateOut> {
 public:
  SolverState() : hh::AbstractState<SStateInNb, SStateIn, SStateOut>() {}

  /* Result *******************************************************************/

  /// @brief Recieves the blocks from the cholesky decomposition graph
  void execute(std::shared_ptr<MatrixBlockData<T, Result>> block) override {
    if (blocks_.size() == 0) {
      nbBlocksCols_ = block->nbBlocksCols();
      nbBlocksRows_ = block->nbBlocksRows();
      blocks_ = std::vector<std::shared_ptr<MatrixBlockData<T, Block>>>(
          block->nbBlocksCols() * block->nbBlocksRows(), nullptr);
    }

    block->rank(0);
    blocks_[block->idx()] = std::make_shared<MatrixBlockData<T, Block>>(std::move(block));
    sendSolveDiagPendings();
    sendUpdatePendings();
  }

  /* Block ********************************************************************/

  /// @brief Recieves the vector blocks from the split matrix task
  void execute(std::shared_ptr<MatrixBlockData<T, Block>> vecBlock) override {
    if (vectorBlocks_.size() == 0) {
      vectorBlocksTtl_ = vecBlock->nbBlocksRows();
      vectorBlocks_ = std::vector<std::shared_ptr<MatrixBlockData<T, Vector>>>(
          vecBlock->nbBlocksRows(), nullptr);
    }

    --vectorBlocksTtl_;
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

  /* Vector *******************************************************************/

  void execute(std::shared_ptr<MatrixBlockData<T, Vector>> block) override {
    blocks_[block->idx()]->incRank();

    for (size_t i = block->y() + 1; i < nbBlocksRows_; ++i) {
      size_t colBlockIdx = i * nbBlocksCols_ + block->y() + 1;
      updateVecPendings_.emplace_back(UpdateVectorIdx(
            colBlockIdx,
            block->idx(),
            i
          ));
    }
    this->addResult(std::make_shared<MatrixBlockData<T, Result>>(block));
    sendUpdatePendings();
  }

  /* Updated ******************************************************************/

  void execute(std::shared_ptr<MatrixBlockData<T, Updated>> block) override {
    blocks_[block->idx()]->incRank();

    if (block->rank() == block->y()) {
      solveDiagPendings_.emplace_back(SolveDiagonalIdx(
            block->y() * nbBlocksCols_ + block->y(),
            block->idx()
          ));
    }
    sendSolveDiagPendings();
  }

  /* idDone ******************************************************************/

  [[nodiscard]] bool isDone() const {
    return vectorBlocks_.size() && vectorBlocks_.back()->isProcessed();
  }

 private:
  struct SolveDiagonalIdx {
    SolveDiagonalIdx(size_t diag, size_t vec):
      diag(diag), vec(vec) {}
    size_t diag; // diagonal element
    size_t vec;  // vector element
  };
  struct UpdateVectorIdx {
    UpdateVectorIdx(size_t col, size_t solvedVec, size_t updatedVec):
      col(col), solvedVec(solvedVec), updatedVec(updatedVec) {}
    size_t col;        // column element
    size_t solvedVec;  // solved vector part
    size_t updatedVec; // the part of the vector that we want to update
  };
  std::vector<std::shared_ptr<MatrixBlockData<T, Block>>> blocks_ = {};
  std::vector<std::shared_ptr<MatrixBlockData<T, Vector>>> vectorBlocks_ = {};
  std::list<SolveDiagonalIdx> solveDiagPendings_ = {};
  std::list<UpdateVectorIdx> updateVecPendings_ = {};
  size_t nbBlocksCols_ = 0;
  size_t nbBlocksRows_ = 0;
  size_t blocksTtl_ = 0;
  size_t vectorBlocksTtl_ = 0;

  void sendUpdatePendings() {
    auto it = updateVecPendings_.begin();

    while (it != updateVecPendings_.end()) {
      auto col = blocks_[it->col];
      auto solvedVec = vectorBlocks_[it->solvedVec];
      auto updatedVec = vectorBlocks_[it->updatedVec];

      if (col) {
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
    auto it = solveDiagPendings_.begin();

    while (it != solveDiagPendings_.end()) {
      auto diag = blocks_[it->diag];
      auto vec = vectorBlocks_[it->diag];

      if (diag) {
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
