#include "graph/cholesky_graph.h"
#include "utils.h"
#include "config.h"
#include <cblas.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#include <tuple>

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
  auto triangular = std::make_shared<MatrixData<T, MatrixTypes::Matrix>>(width, height,
                                                                         config.blockSize,
                                                                         new T[width * height]());
  auto solution = std::make_shared<MatrixData<T, MatrixTypes::Vector>>(1, height, config.blockSize,
                                                                       new T[height]());

  // read the matrix
  for (size_t i = 0; i < width * height; ++i) {
    fs.read(reinterpret_cast<char *>(matrix->get() + i), sizeof(matrix->get()[i]));
  }

  // read the expected triangular matrix and its size
  fs.read(reinterpret_cast<char *>(&width), sizeof(width));
  fs.read(reinterpret_cast<char *>(&height), sizeof(height));
  for (size_t i = 0; i < width * height; ++i) {
    fs.read(reinterpret_cast<char *>(triangular->get() + i), sizeof(triangular->get()[i]));
  }

  // read the result vector and its size
  fs.read(reinterpret_cast<char *>(&width), sizeof(width));
  fs.read(reinterpret_cast<char *>(&height), sizeof(height));
  for (size_t i = 0; i < height; ++i) {
    fs.read(reinterpret_cast<char *>(result->get() + i), sizeof(result->get()[i]));
  }

  // read the solution vector and its size
  fs.read(reinterpret_cast<char *>(&width), sizeof(width));
  fs.read(reinterpret_cast<char *>(&height), sizeof(height));
  for (size_t i = 0; i < height; ++i) {
    fs.read(reinterpret_cast<char *>(solution->get() + i), sizeof(solution->get()[i]));
  }

  return std::make_tuple(matrix, result, triangular, solution);
}

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
          .print = false
  };

  openblas_set_num_threads(1);
  parseCmdArgs(argc, argv, config);

  auto input = initMatrix<MatrixType>(config);
  auto matrix = std::get<0>(input);
  auto result = std::get<1>(input);
  auto triangular = std::get<2>(input);
  auto solution = std::get<3>(input);

  if (config.print) {
    std::cout << "matrix:" << std::endl;
    std::cout << *matrix << std::endl;
  }

  CholeskyGraph<MatrixType> choleskyGraph(
          config.nbThreadsComputeDiagonalTask,
          config.nbThreadsComputeColumnTask,
          config.nbThreadsUpdateTask);

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
    std::cout << "triangular:" << std::endl;
    std::cout << *triangular << std::endl;
    std::cout << "found:" << std::endl;
    std::cout << *matrix << std::endl;
    std::cout << "found solution:" << std::endl;
    std::cout << *result << std::endl;
    std::cout << "expected solution:" << std::endl;
    std::cout << *solution << std::endl;
  }

  choleskyGraph.createDotFile(config.dotFile, hh::ColorScheme::EXECUTION,
                                           hh::StructureOptions::QUEUE);

  if (!verifySolution(matrix, triangular, 1e-3)) {
    std::cout << "ERROR: wrong decomposition" << std::endl;
  }
  if (!verifySolution(result, solution, 1e-3)) {
    std::cout << "ERROR: wrong solution" << std::endl;
  }

  delete[] matrix->get();
  delete[] triangular->get();
  delete[] result->get();
  delete[] solution->get();
  return 0;
}
