#include "graph/cholesky_graph.h"
#include "utils.h"
#include "config.h"
#include <cblas.h>
#include <iostream>
#include <chrono>
#include <tuple>

using MatrixType = double;

/******************************************************************************/
/* main                                                                       */
/******************************************************************************/

int main(int argc, char **argv) {
  Config config = {
          .inputFile = "cholesky.in",
          .dotFile = "cholesky-graph.dot",
          .blockSize = 10,
          .nbThreadsComputeDiagonalTask = 1,
          .nbThreadsComputeColumnTask = 4,
          .nbThreadsUpdateTask = 4,
          .nbThreadsSolveDiagonal = 1,
          .nbThreadsUpdateVector = 4,
          .print = false
  };

  openblas_set_num_threads(1);
  parseCmdArgs(argc, argv, config);

  auto input = initMatrix<MatrixType>(config);
  auto matrix = input.inputMatrix;
  auto result = input.resultVector;
#ifdef TESTING
  auto triangular = input.triangularMatrix;
  auto solution = input.solutionVector;
#endif

  if (config.print) {
    std::cout << "matrix:" << std::endl;
    std::cout << *matrix << std::endl;
  }

  CholeskyGraph<MatrixType> choleskyGraph(
          config.nbThreadsComputeDiagonalTask,
          config.nbThreadsComputeColumnTask,
          config.nbThreadsUpdateTask,
          config.nbThreadsSolveDiagonal,
          config.nbThreadsUpdateVector);

  choleskyGraph.executeGraph(true);

  auto begin = std::chrono::system_clock::now();

  choleskyGraph.pushData(matrix);
  choleskyGraph.pushData(result);
  choleskyGraph.finishPushingData();
  choleskyGraph.waitForTermination();

  auto end = std::chrono::system_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms"
            << std::endl;

  if (config.print) {
    std::cout << "found matrix:" << std::endl;
    std::cout << *matrix << std::endl;
#ifdef TESTING
    std::cout << "expected matrix:" << std::endl;
    std::cout << *triangular << std::endl;
#endif
    std::cout << "found solution:" << std::endl;
    std::cout << *result << std::endl;
#ifdef TESTING
    std::cout << "expected solution:" << std::endl;
    std::cout << *solution << std::endl;
#endif
  }

  choleskyGraph.createDotFile(config.dotFile, hh::ColorScheme::EXECUTION,
                              hh::StructureOptions::QUEUE);

#ifdef TESTING
  if (!verifySolution(matrix, triangular, 1e-3)) {
    std::cout << "ERROR: wrong decomposition" << std::endl;
  }
  if (!verifySolution(result, solution, 1e-3)) {
    std::cout << "ERROR: wrong solution" << std::endl;
  }
#endif

  delete[] matrix->get();
  delete[] result->get();
#ifdef TESTING
  delete[] triangular->get();
  delete[] solution->get();
#endif
  return 0;
}
