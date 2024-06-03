#include "graph/cholesky_decomposition_graph.h"
#include "utils.h"
#include "config.h"
#include <cblas.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>

using MatrixType = double;

template <typename T>
std::pair<std::shared_ptr<MatrixData<T>>, std::shared_ptr<MatrixData<T>>>
initMatrix(Config const &config) {
  std::ifstream fs(config.inputFile);
  size_t width, height;

  // make openblas using only one thread
  openblas_set_num_threads(1);

  fs >> width >> height;
  auto matrix = std::make_shared<MatrixData<T>>(width, height, config.blockSize, new T[width * height]());
  auto expected = std::make_shared<MatrixData<T>>(width, height, config.blockSize, new T[width * height]());

  for (size_t i = 0; i < width * height; ++i) {
    fs >> matrix->get()[i];
  }

  fs >> width >> height;
  for (size_t i = 0; i < width * height; ++i) {
    fs >> expected->get()[i];
  }

  return std::make_pair(matrix, expected);
}

int main(int argc, char **argv) {
  Config config = {
    .inputFile = "cholesky.in",
    .blockSize = 10,
    .nbThreadsComputeColumnTask = 4,
    .nbThreadsUpdateTask = 4,
    .print = false
  };

  parseCmdArgs(argc, argv, config);

  auto input = initMatrix<MatrixType>(config);
  auto matrix = input.first;
  auto expected = input.second;

  if (config.print) {
    std::cout << "matrix:" << std::endl;
    std::cout << *matrix << std::endl;
  }

  CholeskyDecompositionGraph<MatrixType> choleskyGraph(config.nbThreadsComputeColumnTask, config.nbThreadsUpdateTask);
  choleskyGraph.executeGraph(true);

  auto begin = std::chrono::system_clock::now();

  choleskyGraph.pushData(matrix);
  choleskyGraph.finishPushingData();
  choleskyGraph.waitForTermination();

  auto end = std::chrono::system_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

  if (config.print) {
    std::cout << "expected:" << std::endl;
    std::cout << *expected << std::endl;
    std::cout << "found:" << std::endl;
    std::cout << *matrix << std::endl;
  }

  choleskyGraph.createDotFile("cholesky-graph.dot", hh::ColorScheme::EXECUTION,
                              hh::StructureOptions::QUEUE);

  if (!verrifySolution(matrix->width(), matrix->get(), expected->get(), 1e-3)) {
    std::cout << "ERROR" << std::endl;
  }

  delete[] matrix->get();
  delete[] expected->get();
  return 0;
}
