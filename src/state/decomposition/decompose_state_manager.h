#ifndef DECOMPOSE_STATE_MANAGER_H
#define DECOMPOSE_STATE_MANAGER_H
#include "decompose_state.h"
#include "hedgehog/hedgehog/hedgehog.h"

template <typename T>
class DecomposeStateManager
    : public hh::StateManager<DStateInNb, DStateIn, DStateOut> {
public:
  explicit DecomposeStateManager(std::shared_ptr<DecomposeState<T>> const &state)
      : hh::StateManager<DStateInNb, DStateIn, DStateOut>(
            state, "Decompose State Manager") {}

    [[nodiscard]] bool canTerminate() const override {
        this->state()->lock();
        auto ret = std::dynamic_pointer_cast<DecomposeState<T>>(this->state())->isDone();
        this->state()->unlock();
        return ret;
    }
};

#endif
