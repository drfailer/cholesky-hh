#include "data/matrix_data.h"
#include "data/matrix_types.h"
#include "graph/cholesky_graph.h"
#include "utils.h"
#include "config.h"
#include <cblas.h>
#include <iostream>
#include <chrono>
#include <memory>
#include <vector>
#define NB_MEASURES 5

/******************************************************************************/
/* parameters                                                                 */
/******************************************************************************/

using MatrixType = double;

std::vector<ThreadsConfig> threadsConfigs = {
  /* ThreadsConfig(1, 8, 35, 8, 1), */
  /* ThreadsConfig(1, 8, 35, 8, 5), */
  /* ThreadsConfig(1, 8, 35, 8, 10), */
  /* ThreadsConfig(1, 8, 35, 8, 15), */
  /* ThreadsConfig(1, 8, 35, 8, 20), */
  /* ThreadsConfig(1, 8, 35, 8, 25), */
  /* ThreadsConfig(1, 8, 35, 8, 30), */
  /* ThreadsConfig(1, 8, 35, 8, 35), */
  /* ThreadsConfig(1, 8, 35, 8, 40), */

  ThreadsConfig(1, 8, 5, 8, 30),
  ThreadsConfig(1, 8, 10, 8, 30),
  ThreadsConfig(1, 8, 15, 8, 30),
  ThreadsConfig(1, 8, 20, 8, 30),
  ThreadsConfig(1, 8, 25, 8, 30),
  ThreadsConfig(1, 8, 30, 8, 30),
  ThreadsConfig(1, 8, 35, 8, 30),
};

/******************************************************************************/
/* run the algorithm                                                          */
/******************************************************************************/

void cholesky(Config const &config,
              std::shared_ptr<MatrixData<MatrixType, MatrixTypes::Matrix>> matrix,
              std::shared_ptr<MatrixData<MatrixType, MatrixTypes::Vector>> result) {
  CholeskyGraph<MatrixType> choleskyGraph(
          config.threadsConfig.nbThreadsComputeDiagonalTask,
          config.threadsConfig.nbThreadsComputeColumnTask,
          config.threadsConfig.nbThreadsUpdateTask,
          config.threadsConfig.nbThreadsSolveDiagonal,
          config.threadsConfig.nbThreadsUpdateVector);
  choleskyGraph.executeGraph(true);

  /* launch the graph */

  auto begin = std::chrono::system_clock::now();

  choleskyGraph.pushData(matrix);
  choleskyGraph.pushData(result);
  choleskyGraph.finishPushingData();
  choleskyGraph.waitForTermination();

  auto end = std::chrono::system_clock::now();
  std::cout << config.threadsConfig << " "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms"
            << std::endl;

  /* create dot file */
  if (config.dotFile.empty()) {
    choleskyGraph.createDotFile(
        dotFileName(matrix->height(), config.threadsConfig),
        hh::ColorScheme::EXECUTION, hh::StructureOptions::QUEUE);
  } else {
    choleskyGraph.createDotFile(config.dotFile, hh::ColorScheme::EXECUTION,
        hh::StructureOptions::QUEUE);
  }
}

/******************************************************************************/
/* main                                                                       */
/******************************************************************************/

int main(int argc, char **argv) {
  Config config = {
          .inputFile = "cholesky.in",
          .dotFile = "cholesky-graph.dot",
          .blockSize = 10,
          .print = false,
          .loop = false,
          .threadsConfig = ThreadsConfig()
  };

  openblas_set_num_threads(1);
  parseCmdArgs(argc, argv, config);

  auto problem = initMatrix<MatrixType>(config); // matrix allocated

  if (config.loop) {
    for (auto threadsConfig : threadsConfigs) {
      for (size_t nbMeasures = 0; nbMeasures < NB_MEASURES; ++nbMeasures) {
        config.threadsConfig = threadsConfig;
        cholesky(config, problem.matrix, problem.result);
        verifySolution(problem, 1e-3);
        problem.matrix->reset(problem.baseMatrix);
        problem.result->reset(problem.baseResult);
      }
    }
  } else {
    cholesky(config, problem.matrix, problem.result);
    verifySolution(problem, 1e-3);
  }

  print(config, problem);

  free(problem); // matrix freed
  return 0;
}
