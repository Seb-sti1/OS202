#include <algorithm>
#include <cassert>
#include <iostream>
#include <thread>
#if defined(_OPENMP)
#include <omp.h>
#endif
#include "ProdMatMat.hpp"

namespace
{
  void prodSubBlocks(int iRowBlkA, int iColBlkB, int iColBlkA, int szBlock,
                     const Matrix &A, const Matrix &B, Matrix &C)
  {

    // #pragma omp parallel for
    for (int j = iColBlkB; j < std::min(B.nbCols, iColBlkB + szBlock); j++)
      for (int k = iColBlkA; k < std::min(A.nbCols, iColBlkA + szBlock); k++)
        for (int i = iRowBlkA; i < std::min(A.nbRows, iRowBlkA + szBlock); ++i)
          C(i, j) += A(i, k) * B(k, j);
  }
  const int szBlock = 512;
} // namespace

Matrix operator*(const Matrix &A, const Matrix &B)
{
  Matrix C(A.nbRows, B.nbCols, 0.0);

#pragma omp parallel for
  for (int j = 0; j < B.nbCols; j += szBlock)
    for (int k = 0; k < A.nbCols; k += szBlock)
      for (int i = 0; i < A.nbRows; i += szBlock)
        prodSubBlocks(i, j, k, szBlock, A, B, C);

  return C;
}
