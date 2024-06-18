#ifndef UTILS_HPP
#define UTILS_HPP
#define TESTING

#include "data/matrix_data.h"
#include "config.h"
#include <fstream>
#include <memory>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <random>

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

/******************************************************************************/
/* init                                                                       */
/******************************************************************************/

template <typename T>
struct InitType {
  using Matrix = std::shared_ptr<MatrixData<T, MatrixTypes::Matrix>>;
  using Vector = std::shared_ptr<MatrixData<T, MatrixTypes::Vector>>;

  InitType(Matrix inputMatrix,
           Vector resultVector,
           Matrix triangularMatrix = nullptr,
           Vector solutionVector = nullptr) :
          inputMatrix(inputMatrix),
          resultVector(resultVector),
          triangularMatrix(triangularMatrix),
          solutionVector(solutionVector) {}

  Matrix inputMatrix = nullptr;
  Vector resultVector = nullptr;
  Matrix triangularMatrix = nullptr;
  Vector solutionVector = nullptr;
};

template <typename T>
InitType<T> initMatrix(Config const &config) {
  std::ifstream fs(config.inputFile, std::ios::binary);
  size_t width, height;

  // read the size of the matrix
  fs.read(reinterpret_cast<char *>(&width), sizeof(width));
  fs.read(reinterpret_cast<char *>(&height), sizeof(height));

  auto matrix = std::make_shared<MatrixData<T, MatrixTypes::Matrix>>(
          width, height, config.blockSize, new T[width * height]());
  auto result = std::make_shared<MatrixData<T, MatrixTypes::Vector>>(
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
  }

  // read the result vector
  for (size_t i = 0; i < height; ++i) {
    fs.read(reinterpret_cast<char *>(result->get() + i), sizeof(result->get()[i]));
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
  return InitType(matrix, result, triangular, solution);
#else
  return InitType(matrix, result);
#endif
}

#endif
