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

#ifndef CHOLESKY_SOLVER_GRAPH_H
#define CHOLESKY_SOLVER_GRAPH_H

#include "../data/matrix_block_data.h"
#include "../data/matrix_data.h"
#include "../task/decomposition/split_matrix_task.h"
#include "../task/solver/solve_diagonal_task.h"
#include "../task/solver/update_vector_task.h"
#include "../state/solver/solver_state.h"
#include "../state/solver/solver_state_manager.h"
#include <hedgehog/hedgehog.h>

#define CSGraphInNb 3
#define CSGraphIn                        \
  MatrixBlockData<T, VectorBlock>,       \
  MatrixBlockData<T, VectorBlockPhase1>, \
  MatrixBlockData<T, Decomposed>
#define CSGraphOut MatrixBlockData<T, VectorBlockPhase1>, MatrixBlockData<T, Result>

template <typename T, Phases Phase>
class CholeskySolverGraph
        : public hh::Graph<CSGraphInNb, CSGraphIn, CSGraphOut > {
 public:
  CholeskySolverGraph(size_t nbThreadsSolveDiagonal, size_t nbThreadsUpdateVector)
          : hh::Graph<CSGraphInNb, CSGraphIn, CSGraphOut >(Phase == Phases::First
                                                           ? "Cholesky Solver phase 1"
                                                           : "Cholesky Solver phase 2") {
    auto solveDiagonalTask = std::make_shared<SolveDiagonalTask<T, Phase>>(nbThreadsSolveDiagonal);
    auto updateVectorTask = std::make_shared<UpdateVectorTask<T, Phase>>(nbThreadsUpdateVector);
    auto solverState = std::make_shared<SolverState<T, Phase>>();
    auto solverStateManager = std::make_shared<SolverStateManager<T, Phase>>(solverState);

    this->inputs(solverStateManager);

    this->edges(solverStateManager, solveDiagonalTask);
    this->edges(solveDiagonalTask, solverStateManager);

    this->edges(solverStateManager, updateVectorTask);
    this->edges(updateVectorTask, solverStateManager);

    this->outputs(solverStateManager);
  }
};

#endif
