#include "graph/cholesky_decomposition_graph.h"
#include "graph/cholesky_solver_graph.h"
#include "utils.h"
#include "config.h"
#include <cblas.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#define TESTING

using MatrixType = double;

template <typename T>
std::pair<std::shared_ptr<MatrixData<T>>, std::shared_ptr<MatrixData<T>>>
initMatrix(Config const &config) {
  std::ifstream fs(config.inputFile, std::ios::binary);
  size_t width, height;

  fs.read(reinterpret_cast<char*>(&width), sizeof(width));
  fs.read(reinterpret_cast<char*>(&height), sizeof(height));
  auto matrix = std::make_shared<MatrixData<T>>(width, height,
      config.blockSize, new T[width * height]());
#ifdef TESTING
  auto expected = std::make_shared<MatrixData<T>>(width, height,
      config.blockSize, new T[width * height]());
#endif

  for (size_t i = 0; i < width * height; ++i) {
    fs.read(reinterpret_cast<char*>(matrix->get() + i), sizeof(matrix->get()[i]));
  }

#ifdef TESTING
  fs.read(reinterpret_cast<char*>(&width), sizeof(width));
  fs.read(reinterpret_cast<char*>(&height), sizeof(height));
  for (size_t i = 0; i < width * height; ++i) {
    fs.read(reinterpret_cast<char*>(expected->get() + i), sizeof(expected->get()[i]));
  }
#endif

#ifdef TESTING
  return std::make_pair(matrix, expected);
#else
  return std::make_pair(matrix, nullptr);
#endif
}

int main(int argc, char **argv) {
  Config config = {
    .inputFile = "cholesky.in",
    .dotFile = "cholesky-graph.dot",
    .blockSize = 10,
    .nbThreadsComputeDiagonalTask = 1,
    .nbThreadsComputeColumnTask = 4,
    .nbThreadsUpdateTask = 4,
    .print = false
  };

  openblas_set_num_threads(1);
  parseCmdArgs(argc, argv, config);

  auto input = initMatrix<MatrixType>(config);
  auto matrix = input.first;
#ifdef TESTING
  auto expected = input.second;
#endif

  if (config.print) {
    std::cout << "matrix:" << std::endl;
    std::cout << *matrix << std::endl;
  }

  CholeskyDecompositionGraph<MatrixType> choleskyGraph(
      config.nbThreadsComputeDiagonalTask,
      config.nbThreadsComputeColumnTask,
      config.nbThreadsUpdateTask);
  CholeskySolverGraph<MatrixType> choleskySolverGraph;

  choleskySolverGraph.executeGraph(true);
  choleskyGraph.executeGraph(true);

  auto begin = std::chrono::system_clock::now();

  choleskyGraph.pushData(matrix);
  choleskyGraph.finishPushingData();
  choleskyGraph.waitForTermination();

  auto end = std::chrono::system_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

  if (config.print) {
#ifdef TESTING
    std::cout << "expected:" << std::endl;
    std::cout << *expected << std::endl;
#endif
    std::cout << "found:" << std::endl;
    std::cout << *matrix << std::endl;
  }

  choleskyGraph.createDotFile(config.dotFile, hh::ColorScheme::EXECUTION,
                              hh::StructureOptions::QUEUE);

#ifdef TESTING
  if (!verrifySolution(matrix->width(), matrix->get(), expected->get(), 1e-3)) {
    std::cout << "ERROR" << std::endl;
  }
#endif

  delete[] matrix->get();
#ifdef TESTING
  delete[] expected->get();
#endif
  return 0;
}
