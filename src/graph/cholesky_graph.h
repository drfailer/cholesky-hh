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

#ifndef CHOLESKY_HH_CHOLESKY_GRAPH_H
#define CHOLESKY_HH_CHOLESKY_GRAPH_H

#include <hedgehog/hedgehog.h>
#include "../data/matrix_data.h"
#include "../data/matrix_block_data.h"
#include "cholesky_decomposition_graph.h"
#include "cholesky_solver_graph.h"

#define CGraphInNb 2
#define CGraphIn MatrixData<T, MatrixTypes::Matrix>, MatrixData<T, MatrixTypes::Vector>
#define CGraphOut MatrixBlockData<T, Result>

template <typename T>
class CholeskyGraph
        : public hh::Graph<CGraphInNb, CGraphIn, CGraphOut > {
 public:
  CholeskyGraph(size_t nbThreadsComputeDiagonalTask,
                size_t nbThreadsComputeColumnTask,
                size_t nbThreadsUpdateTask,
                size_t nbThreadsSolveDiagonal,
                size_t nbThreadsUpdateVector)
          : hh::Graph<CGraphInNb, CGraphIn, CGraphOut >("Cholesky") {
    auto splitTask = std::make_shared<SplitMatrixTask<T>>();
    auto choleskyDecompositionGraph = std::make_shared<CholeskyDecompositionGraph<T>>(
            nbThreadsComputeDiagonalTask,
            nbThreadsComputeColumnTask,
            nbThreadsUpdateTask);
    auto choleskySolverGraph1 =
            std::make_shared<CholeskySolverGraph<T, Phases::First>>(
                    nbThreadsSolveDiagonal, nbThreadsUpdateVector);
    auto choleskySolverGraph2 =
            std::make_shared<CholeskySolverGraph<T, Phases::Second>>(
                    nbThreadsSolveDiagonal, nbThreadsUpdateVector);

    this->inputs(splitTask);

    this->edges(splitTask, choleskyDecompositionGraph);
    this->edges(splitTask, choleskySolverGraph1);

    this->edges(choleskySolverGraph1, choleskySolverGraph2);
    this->edges(choleskyDecompositionGraph, choleskySolverGraph1);
    this->edges(choleskyDecompositionGraph, choleskySolverGraph2);

    this->outputs(choleskySolverGraph2);
  }
};

#endif //CHOLESKY_HH_CHOLESKY_GRAPH_H
