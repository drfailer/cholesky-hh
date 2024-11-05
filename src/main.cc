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
#define NB_MEASURES 10

using MatrixType = double;

/******************************************************************************/
/* threads config                                                             */
/******************************************************************************/

std::vector<ThreadsConfig> threadsConfigs = {
  // place some values
};

void initThreadsConfig() {
  threadsConfigs.clear();

  for (size_t i = 5; i <= 40; i += 5) {
    threadsConfigs.push_back(ThreadsConfig(1, 8, i, 8, 30));
  }
}

/******************************************************************************/
/* run the algorithm                                                          */
/******************************************************************************/

void cholesky(Config const &config,
              std::shared_ptr<MatrixData<MatrixType, MatrixTypes::Matrix>> &matrix,
              std::shared_ptr<MatrixData<MatrixType, MatrixTypes::Vector>> &result) {
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
  std::cout << matrix->height() << " " << matrix->blockSize() << " " << config.threadsConfig << " "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms"
            << std::endl;

  /* create dot file */
  if (!config.dotFile.ends_with(".dot")) {
    std::string dotFileNameStr = config.dotFile + "/"
      + dotFileName(matrix->height(), matrix->blockSize(), config.threadsConfig);
    choleskyGraph.createDotFile(dotFileNameStr, hh::ColorScheme::EXECUTION,
        hh::StructureOptions::QUEUE);
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
  initThreadsConfig();

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
