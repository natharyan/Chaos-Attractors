#ifndef MATRIX_H
#define MATRIX_H

#include <vector>

class Matrix {
public:
    Matrix(size_t rows, size_t cols);
    double& operator()(size_t i, size_t j);
    double operator()(size_t i, size_t j) const;
    
    size_t mRows;
    size_t mCols;
    std::vector<double> mData;
};

Matrix matrix_multiplication(const Matrix& a, const Matrix& b);

#endif