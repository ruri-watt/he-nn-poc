//
// Created by rw on 4/7/21.
//

#ifndef HE_NN_EXPLORATION_MATRIX_H
#define HE_NN_EXPLORATION_MATRIX_H

#include <array>
#include <ostream>

using namespace std;
template<class T>
using matrix = vector<vector<T>>;


template<class T>
vector<T> operator*(const matrix<T> &a, vector<T> &b) {
    vector<T> result;
    for (size_t r = 0; r < a.size(); r++) {
        if (a[0].size() != b.size()) {
            return result;
        }
        result.resize(a.size());
            for (size_t i = 0; i < b.size(); i++) {
                result[r] += a[r][i] * b[i];
            }
    }
    return result;
}

template<class T>
matrix<T> operator*(const matrix<T> &a, matrix<T> &b) {
    matrix<T> result;
    for (size_t r = 0; r < a.size(); r++) {
        if (a[0].size() != b.size()) {
            return result;
        }
        result.resize(a.size());
        for (auto &row: result) {
            row.resize(b[0].size());
        }
        for (size_t c = 0; c < b[0].size(); c++) {
            for (size_t i = 0; i < b.size(); i++) {
                result[r][c] += a[r][i] * b[i][c];
            }
        }
    }
    return result;
}

template<class T>
std::ostream &operator<<(std::ostream &os, const matrix<T> &matrix) {
    for (const auto & row : matrix) {
        for (const auto val : row) {
            os << val << " ";
        }
        os << '\n';
    }
    return os;
}

#endif //HE_NN_EXPLORATION_MATRIX_H
