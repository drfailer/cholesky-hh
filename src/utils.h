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

#ifndef UTILS_HPP
#define UTILS_HPP
#define TESTING

#include "data/matrix_data.h"
#include "config.h"
#include <fstream>
#include <memory>
#include <cstddef>
#include <cstring>
#include <iostream>

/******************************************************************************/
/* init                                                                       */
/******************************************************************************/

template <typename T>
struct Problem {
  using Matrix = std::shared_ptr<MatrixData<T, MatrixTypes::Matrix>>;
  using Vector = std::shared_ptr<MatrixData<T, MatrixTypes::Vector>>;

  Problem(Matrix inputMatrix,
          Vector resultVector,
          Matrix saveInputMatrix,
          Vector saveResultVector,
          Matrix triangularMatrix = nullptr,
          Vector solutionVector = nullptr) :
    matrix(inputMatrix),
    result(resultVector),
    baseMatrix(saveInputMatrix),
    baseResult(saveResultVector),
    expectedMatrix(triangularMatrix),
    expectedSolution(solutionVector) {}

  Matrix matrix = nullptr;
  Vector result = nullptr;
  Matrix baseMatrix = nullptr;
  Vector baseResult = nullptr;
  Matrix expectedMatrix = nullptr;
  Vector expectedSolution = nullptr;
};

template <typename T>
Problem<T> initMatrix(Config const &config) {
  std::ifstream fs(config.inputFile, std::ios::binary);
  size_t width, height;

  // read the size of the matrix
  fs.read(reinterpret_cast<char *>(&width), sizeof(width));
  fs.read(reinterpret_cast<char *>(&height), sizeof(height));

  auto matrix = std::make_shared<MatrixData<T, MatrixTypes::Matrix>>(
          width, height, config.blockSize, new T[width * height]());
  auto result = std::make_shared<MatrixData<T, MatrixTypes::Vector>>(
          1, height, config.blockSize, new T[height]());
  auto saveMatrix = std::make_shared<MatrixData<T, MatrixTypes::Matrix>>(
          width, height, config.blockSize, new T[width * height]());
  auto saveResult = std::make_shared<MatrixData<T, MatrixTypes::Vector>>(
          1, height, config.blockSize, new T[height]());
#ifdef TESTING
  auto triangular = std::make_shared<MatrixData<T, MatrixTypes::Matrix>>(
          width, height, config.blockSize, new T[width * height]());
  auto solution = std::make_shared<MatrixData<T, MatrixTypes::Vector>>(
          1, height, config.blockSize, new T[height]());
#endif

  // read the symmetric matrix
  for (size_t i = 0; i < width * height; ++i) {
    fs.read(reinterpret_cast<char *>(matrix->get() + i), sizeof(matrix->get()[i]));
    saveMatrix->get()[i] = matrix->get()[i];
  }

  // read the result vector
  for (size_t i = 0; i < height; ++i) {
    fs.read(reinterpret_cast<char *>(result->get() + i), sizeof(result->get()[i]));
    saveResult->get()[i] = result->get()[i];
  }

#ifdef TESTING
  // read the expected triangular matrix
  for (size_t i = 0; i < width * height; ++i) {
    fs.read(reinterpret_cast<char *>(triangular->get() + i), sizeof(triangular->get()[i]));
  }

  // read the solution vector
  for (size_t i = 0; i < height; ++i) {
    fs.read(reinterpret_cast<char *>(solution->get() + i), sizeof(solution->get()[i]));
  }
#endif

#ifdef TESTING
  return Problem(matrix, result, saveMatrix, saveResult, triangular, solution);
#else
  return InitType(matrix, result, saveMatrix, saveResult);
#endif
}

template <typename T>
void free(Problem<T> &problem) {
  delete[] problem.matrix->get();
  delete[] problem.result->get();
  delete[] problem.baseMatrix->get();
  delete[] problem.baseResult->get();
  if (problem.expectedMatrix) {
    delete[] problem.expectedMatrix->get();
  }
  if (problem.expectedSolution) {
    delete[] problem.expectedSolution->get();
  }
}

/******************************************************************************/
/* test functions                                                             */
/******************************************************************************/

template <typename Type>
bool verifySolution(std::shared_ptr<MatrixData<Type, MatrixTypes::Matrix>> founded,
                    std::shared_ptr<MatrixData<Type, MatrixTypes::Matrix>> expected,
                    Type precision) {
  bool output = true;

  for (size_t i = 0; i < founded->height(); ++i) {
    for (size_t j = 0; j <= i; ++j) {
      if (!((founded->at(i, j) - precision) <= expected->at(i, j) &&
            expected->at(i, j) <= (founded->at(i, j) + precision))) {
        output = false;
      }
    }
  }
  return output;
}

template <typename Type>
bool verifySolution(std::shared_ptr<MatrixData<Type, MatrixTypes::Vector>> founded,
                    std::shared_ptr<MatrixData<Type, MatrixTypes::Vector>> expected,
                    Type precision) {
  bool output = true;

  for (size_t i = 0; i < founded->height(); ++i) {
    if (!((founded->at(i, 0) - precision) <= expected->at(i, 0) &&
          expected->at(i, 0) <= (founded->at(i, 0) + precision))) {
      output = false;
    }
  }
  return output;
}

template <typename Type>
void verifySolution(Problem<Type> const &problem, Type precision) {
#ifdef TESTING
  if (!verifySolution(problem.matrix, problem.expectedMatrix, precision)) {
    std::cerr << "ERROR: wrong decomposition" << std::endl;
  }
  if (!verifySolution(problem.result, problem.expectedSolution, precision)) {
    std::cerr << "ERROR: wrong solution" << std::endl;
  }
#endif
}

/******************************************************************************/
/* print                                                                      */
/******************************************************************************/

template <typename T>
void print(Config const &config, Problem<T> const &problem) {
  if (config.print) {
    std::cout << "base matrix:" << std::endl;
    std::cout << problem.baseMatrix << std::endl;
    std::cout << "base result:" << std::endl;
    std::cout << problem.baseResult << std::endl;
    std::cout << "found matrix:" << std::endl;
    std::cout << problem.matrix << std::endl;
    std::cout << "expected matrix:" << std::endl;
    std::cout << problem.expectedMatrix << std::endl;
    std::cout << "found solution:" << std::endl;
    std::cout << problem.result << std::endl;
    std::cout << "expected solution:" << std::endl;
    std::cout << problem.expectedSolution << std::endl;
  }
}

#endif
