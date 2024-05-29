#include "graph/cholesky_decomposition_graph.h"
#include "utils.h"

using MatrixType = double;

int main(int, char **) {
  constexpr size_t size = 10;
  constexpr size_t blockSize = 3;
//  MatrixType matrixMem[size * size] = {1,1,1,1,1,2,2,2,1,2,3,3,1,2,3,4};
//  MatrixType resultMem[size * size] = {1,0,0,0,1,1,0,0,1,1,1,0,1,1,1,1};
  MatrixType matrixMem[size * size];
  MatrixType resultMem[size * size];

  auto matrix = std::make_shared<MatrixData<MatrixType>>(size, size, blockSize,
                                                         matrixMem);
  auto expected = std::make_shared<MatrixData<MatrixType>>(size, size, blockSize,
                                                           resultMem);

  generateRandomCholeskyMatrix(matrix, expected);

  std::cout << "matrix:" << std::endl;
  std::cout << *matrix << std::endl;
  std::cout << "expected:" << std::endl;
  std::cout << *expected << std::endl;

  CholeskyDecompositionGraph<MatrixType> choleskyGraph;
  choleskyGraph.executeGraph();

  choleskyGraph.pushData(matrix);
  choleskyGraph.finishPushingData();
  choleskyGraph.waitForTermination();

  std::cout << "expected:" << std::endl;
  std::cout << *expected << std::endl;
  std::cout << "found:" << std::endl;
  std::cout << *matrix << std::endl;

  choleskyGraph.createDotFile("cholesky-graph.dot", hh::ColorScheme::EXECUTION, hh::StructureOptions::QUEUE);
  assert(verrifySolution(size, matrixMem, resultMem, 1e-3));
  return 0;
}
