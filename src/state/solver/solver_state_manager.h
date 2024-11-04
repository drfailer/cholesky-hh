#ifndef SOLVER_STATE_MANAGER_H
#define SOLVER_STATE_MANAGER_H
#include "solver_state.h"
#include <hedgehog/hedgehog.h>

template <typename T, Phases Phase>
class SolverStateManager
    : public hh::StateManager<SStateInNb, SStateIn, SStateOut> {
public:
  explicit SolverStateManager(std::shared_ptr<SolverState<T, Phase>> const &state)
      : hh::StateManager<SStateInNb, SStateIn, SStateOut>(
            state, "Solver State Manager") {}

    [[nodiscard]] bool canTerminate() const override {
        this->state()->lock();
        auto ret = std::dynamic_pointer_cast<SolverState<T, Phase>>(this->state())->isDone();
        this->state()->unlock();
        return ret;
    }
};

#endif
