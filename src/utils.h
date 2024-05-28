#ifndef UTILS_HPP
#define UTILS_HPP

#include "data/matrix_data.h"
#include <chrono>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <random>

#define timerStart() auto _start = std::chrono::system_clock::now();
#define timerEnd() auto _end = std::chrono::system_clock::now();
#define timerCount()                                                           \
  std::chrono::duration_cast<std::chrono::microseconds>(_end - _start)         \
          .count() /                                                           \
      1.0e6

/******************************************************************************/
/*                              generate problem                              */
/******************************************************************************/

template<typename T>
void cpyTranspose(std::shared_ptr<MatrixData<T>> dest,
                  std::shared_ptr<MatrixData<T>> src) {
  for (size_t i = 0; i < src->height(); ++i) {
    for (size_t j = 0; j <= i; ++j) {
      dest->get()[j * src->width() + i] = src->get()[i * src->width() + j];
    }
  }
}

/// @brief Compute A = L * LT
template<typename T>
void matrixDotProduct(std::shared_ptr<MatrixData<T>> L,
                      std::shared_ptr<MatrixData<T>> LT,
                      std::shared_ptr<MatrixData<T>> A) {
  for (size_t i = 0; i < L->height(); ++i) {
    for (size_t j = 0; j < L->width(); ++j) {
      for (size_t k = 0; k < LT->height(); ++k) {
        A->get()[i * A->width() + j] +=
                L->get()[i * L->width() + k] * LT->get()[k * LT->width() + j];
      }
    }
  }
}

template<typename T>
void generateRandomCholeskyMatrix(std::shared_ptr<MatrixData<T>> matrix,
                                  std::shared_ptr<MatrixData<T>> result) {
  std::random_device dv;
  std::mt19937 gen(dv());
  /* std::uniform_real_distribution<> dis(-10, 10); */
  std::uniform_int_distribution<> dis(0, 10);

  memset(matrix->get(), 0, sizeof(T) * matrix->height() * matrix->width());
  memset(result->get(), 0, sizeof(T) * result->height() * result->width());
  for (size_t i = 0; i < matrix->height(); ++i) {
    for (size_t j = 0; j <= i; ++j) {
      T value = dis(gen);
      if (value == 0)
        value++; // make sure we don't have 0 on the diagonal
      result->get()[i * matrix->width() + j] = value;
    }
  }

  T *resultTMem = new T[matrix->height() * matrix->width()];
  auto resultT = std::make_shared<MatrixData<T>>(matrix->height(), matrix->width(), 1, resultTMem);
  cpyTranspose(resultT, result);
  matrixDotProduct(result, resultT, matrix);
  delete[] resultTMem;
}

/******************************************************************************/
/*                               test functions                               */
/******************************************************************************/

template<typename Type>
bool verrifySolution(size_t size, Type *founded, Type *expected,
                     Type precision) {
  bool output = true;

  for (size_t i = 0; i < size; ++i) {
    for (size_t j = 0; j <= i; ++j) {
      if (!((founded[i * size + j] - precision) <= expected[i * size + j] &&
            expected[i * size + j] <= (founded[i * size + j] + precision))) {
        output = false;
        std::cout << "Error: " << expected[i * size + j]
                  << " != " << founded[i * size + j] << std::endl;
      }
    }
  }
  return output;
}

#endif
