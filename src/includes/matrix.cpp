#include "matrix.h"

Matrix::Matrix(size_t rows, size_t cols)
    : mRows(rows), mCols(cols), mData(rows * cols)
{
}

double& Matrix::operator()(size_t i, size_t j)
{
    return mData[i * mCols + j];
}

double Matrix::operator()(size_t i, size_t j) const
{
    return mData[i * mCols + j];
}

Matrix matrix_multiplication(const Matrix& a, const Matrix& b){
    Matrix result(a.mRows, b.mCols);
    for (size_t i = 0; i < a.mRows; i++)
    {
        for (size_t j = 0; j < b.mCols; j++)
        {
            double sum = 0;
            for (size_t k = 0; k < a.mCols; k++)
            {
                sum += a(i, k) * b(k, j);
            }
            result(i, j) = sum;
        }
    }
    return result;
}