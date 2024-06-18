#ifndef CHOLESKY_HH_UPDATE_SUBMATRIX_STATE_MANAGER_H
#define CHOLESKY_HH_UPDATE_SUBMATRIX_STATE_MANAGER_H

#include "update_submatrix_state.h"
#include "hedgehog/hedgehog/hedgehog.h"

template <typename T>
class UpdateSubMatrixStateManager
        : public hh::StateManager<USMStateInNb, USMStateIn, USMStateOut> {
 public:
  explicit UpdateSubMatrixStateManager(std::shared_ptr<UpdateSubMatrixState<T>> const &state)
          : hh::StateManager<USMStateInNb, USMStateIn, USMStateOut>(
          state, "Update Submatrix State Manager") {}

  [[nodiscard]] bool canTerminate() const override {
    this->state()->lock();
    auto ret = std::dynamic_pointer_cast<UpdateSubMatrixState<T>>(this->state())->isDone();
    this->state()->unlock();
    return ret;
  }

};

#endif //CHOLESKY_HH_UPDATE_SUBMATRIX_STATE_MANAGER_H
