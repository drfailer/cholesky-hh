#include "graph/cholesky_decomposition_graph.h"

using MatrixType = double;

int main(int, char**) {
  CholeskyDecompositionGraph<MatrixType> choleskyGraph;
  choleskyGraph.executeGraph();
  return 0;
}
