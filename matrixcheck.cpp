#include <iostream>
#include <helpers/matrix.h>

int main() {
    Matrix a(2, 2);
    a(0, 0) = 1; a(0, 1) = 2;
    a(1, 0) = 3; a(1, 1) = 4;

    Matrix b(2, 2);
    b(0, 0) = 5; b(0, 1) = 6;
    b(1, 0) = 7; b(1, 1) = 8;

    Matrix c = matrix_multiplication(a, b);

    std::cout << "Result of matrix multiplication:" << std::endl;
    for (size_t i = 0; i < c.mRows; ++i) {
        for (size_t j = 0; j < c.mCols; ++j) {
            std::cout << c(i, j) << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}