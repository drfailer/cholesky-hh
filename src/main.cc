#include "graph/cholesky_graph.h"
#include "utils.h"
#include "config.h"
#include <cblas.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#include <tuple>

#define TESTING

using MatrixType = double;

template <typename T>
using InitType =
        std::tuple<
                std::shared_ptr<MatrixData<T, MatrixTypes::Matrix>>,
                std::shared_ptr<MatrixData<T, MatrixTypes::Vector>>,
                std::shared_ptr<MatrixData<T, MatrixTypes::Matrix>>,
                std::shared_ptr<MatrixData<T, MatrixTypes::Vector>>>;

template <typename T>
InitType<T> initMatrix(Config const &config) {
  std::ifstream fs(config.inputFile, std::ios::binary);
  size_t width, height;

  fs.read(reinterpret_cast<char *>(&width), sizeof(width));
  fs.read(reinterpret_cast<char *>(&height), sizeof(height));
  auto matrix = std::make_shared<MatrixData<T, MatrixTypes::Matrix>>(width, height,
                                                                     config.blockSize,
                                                                     new T[width * height]());
  auto result = std::make_shared<MatrixData<T, MatrixTypes::Vector>>(1, height, config.blockSize,
                                                                     new T[height]());
#ifdef TESTING
  auto triangular = std::make_shared<MatrixData<T, MatrixTypes::Matrix>>(width, height,
                                                                         config.blockSize,
                                                                         new T[width * height]());
  auto solution = std::make_shared<MatrixData<T, MatrixTypes::Vector>>(1, height, config.blockSize,
                                                                       new T[height]());
#endif

  for (size_t i = 0; i < width * height; ++i) {
    fs.read(reinterpret_cast<char *>(matrix->get() + i), sizeof(matrix->get()[i]));
  }

#ifdef TESTING
  fs.read(reinterpret_cast<char *>(&width), sizeof(width));
  fs.read(reinterpret_cast<char *>(&height), sizeof(height));
  for (size_t i = 0; i < width * height; ++i) {
    fs.read(reinterpret_cast<char *>(triangular->get() + i), sizeof(triangular->get()[i]));
  }
#endif

  fs.read(reinterpret_cast<char *>(&width), sizeof(width));
  fs.read(reinterpret_cast<char *>(&height), sizeof(height));
  for (size_t i = 0; i < height; ++i) {
    fs.read(reinterpret_cast<char *>(result->get() + i), sizeof(result->get()[i]));
  }

#ifdef TESTING
  fs.read(reinterpret_cast<char *>(&width), sizeof(width));
  fs.read(reinterpret_cast<char *>(&height), sizeof(height));
  for (size_t i = 0; i < height; ++i) {
    fs.read(reinterpret_cast<char *>(solution->get() + i), sizeof(solution->get()[i]));
  }
#endif

#ifdef TESTING
  return std::make_tuple(matrix, result, triangular, solution);
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
  auto matrix = std::get<0>(input);
  auto result = std::get<1>(input);
#ifdef TESTING
  auto triangular = std::get<2>(input);
  auto solution = std::get<3>(input);
#endif

  if (config.print) {
    std::cout << "matrix:" << std::endl;
    std::cout << *matrix << std::endl;
  }

  CholeskyGraph<MatrixType> choleskyGraph(
          config.nbThreadsComputeDiagonalTask,
          config.nbThreadsComputeColumnTask,
          config.nbThreadsUpdateTask);

  choleskyGraph.executeGraph(true);
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
#ifdef TESTING
    std::cout << "triangular:" << std::endl;
    std::cout << *triangular << std::endl;
#endif
    std::cout << "found:" << std::endl;
    std::cout << *matrix << std::endl;
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
  if (!verrifySolution(matrix->width(), matrix->get(), triangular->get(), 1e-3)) {
    std::cout << "ERROR: wrong decomposition" << std::endl;
  }
  if (!verrifySolution(result->height(), result->get(), solution->get(), 1e-3)) {
    std::cout << "ERROR: wrong solution" << std::endl;
  }
#endif

  delete[] matrix->get();
#ifdef TESTING
  delete[] triangular->get();
#endif
  return 0;
}
