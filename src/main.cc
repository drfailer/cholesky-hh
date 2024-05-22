#include "graph/cholesky_decomposition_graph.h"
#include "utils.h"

using MatrixType = double;

int main(int, char**) {
  constexpr size_t size = 5;
  MatrixType matrixMem[size * size];
  MatrixType resultMem[size * size];

  generateRandomCholeskyMatrix(size, matrixMem, resultMem);

  auto matrix = std::make_shared<MatrixData<MatrixType>>(size, size, 1,
      matrixMem);
  auto expected = std::make_shared<MatrixData<MatrixType>>(size, size, 1,
      resultMem);

  std::cout << "matrix:" << std::endl;
  std::cout << *matrix << std::endl;

  CholeskyDecompositionGraph<MatrixType> choleskyGraph;
  choleskyGraph.executeGraph();

  choleskyGraph.pushData(matrix);
  choleskyGraph.finishPushingData();
  choleskyGraph.waitForTermination();

  std::cout << "expected:" << std::endl;
  std::cout << *expected << std::endl;
  std::cout << "found:" << std::endl;
  std::cout << *matrix << std::endl;
  return 0;
}
