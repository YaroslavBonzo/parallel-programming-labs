#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <format>
#include <random>
#include <stdexcept>
#include <complex>
#include <chrono>
#include <cuda_runtime.h>

using namespace std;

template <typename T>
__global__ void matmul_kernel_naive(const T* A, const T* B, T* C, size_t N)
{
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < N && col < N)
    {
        T sum = 0;
        for (int k = 0; k < N; ++k)
            sum += A[row * N + k] * B[k * N + col];
        C[row * N + col] = sum;
    }
}

template <typename T>
__global__ void matmul_kernel_shared(const T* A, const T* B, T* C, size_t N)
{
    __shared__ T shared_A[32][32];
    __shared__ T shared_B[32][32];

    int bx = blockIdx.x, by = blockIdx.y;
    int tx = threadIdx.x, ty = threadIdx.y;

    int row = by * blockDim.y + ty;
    int col = bx * blockDim.x + tx;

    T sum = 0;

    int num_tiles = (N + blockDim.x - 1) / blockDim.x;

    for (int tile = 0; tile < num_tiles; ++tile)
    {
        if (row < N && tile * blockDim.x + tx < N)
            shared_A[ty][tx] = A[row * N + tile * blockDim.x + tx];
        else
            shared_A[ty][tx] = 0;

        if (col < N && tile * blockDim.y + ty < N)
            shared_B[ty][tx] = B[(tile * blockDim.y + ty) * N + col];
        else
            shared_B[ty][tx] = 0;

        __syncthreads();

        for (int k = 0; k < blockDim.x; ++k)
            sum += shared_A[ty][k] * shared_B[k][tx];

        __syncthreads();
    }

    if (row < N && col < N)
        C[row * N + col] = sum;
}

template <class T>
struct CudaStats
{
    Matrix<T> matrix;
    std::chrono::microseconds duration = std::chrono::microseconds(0);
    dim3 grid_size;
    dim3 block_size;
    bool used_shared_mem = false;
    size_t matrix_size;
};

template <typename T>
class Matrix {
private:
    T* _Matrixptr;
    size_t _lines;
    size_t _columns;
    inline static const double epsilon = 0.001;

public:
    Matrix() : _Matrixptr(nullptr), _lines(0), _columns(0) {}

    Matrix(size_t lines, size_t columns, T value) : _lines(lines), _columns(columns) {
        _Matrixptr = new T[lines * columns];
        for (size_t i = 0; i < lines * columns; i++) {
            _Matrixptr[i] = value;
        }
    }

    Matrix(size_t lines, size_t columns, int inf, int sup) : _lines(lines), _columns(columns) {
        _Matrixptr = new T[lines * columns];
        std::random_device engine;
        std::uniform_int_distribution<int> distribution(std::min(inf, sup), std::max(inf, sup));
        for (size_t i = 0; i < columns * lines; i++) {
            _Matrixptr[i] = distribution(engine);
        }
    }

    Matrix(size_t lines, size_t columns, float inf, float sup) : _lines(lines), _columns(columns) {
        _Matrixptr = new T[lines * columns];
        std::random_device engine;
        std::uniform_real_distribution<float> distribution(std::min(inf, sup), std::max(inf, sup));
        for (size_t i = 0; i < columns * lines; i++) {
            _Matrixptr[i] = distribution(engine);
        }
    }

    Matrix(size_t lines, size_t columns, double inf, double sup) : _lines(lines), _columns(columns) {
        _Matrixptr = new T[lines * columns];
        std::random_device engine;
        std::uniform_real_distribution<double> distribution(std::min(inf, sup), std::max(inf, sup));
        for (size_t i = 0; i < columns * lines; i++) {
            _Matrixptr[i] = distribution(engine);
        }
    }

    Matrix(const Matrix& other) : _lines(other._lines), _columns(other._columns) {
        if (other._Matrixptr == nullptr) {
            _Matrixptr = nullptr;
        }
        else {
            _Matrixptr = new T[_lines * _columns];
            for (size_t i = 0; i < other._lines * other._columns; i++) {
                _Matrixptr[i] = other._Matrixptr[i];
            }
        }
    }

    Matrix<T> multiply_cpu(const Matrix<T>& rhs) const {
        if (_columns != rhs._lines) {
            throw invalid_argument("Uncorrect size of matrix for multiply");
        }
        Matrix<T> result(_lines, rhs._columns, 0);
        for (size_t i = 0; i < _lines; i++) {
            for (size_t j = 0; j < rhs._columns; j++) {
                for (size_t k = 0; k < _columns; k++) {
                    result(i, j) += this->operator()(i, k) * rhs(k, j);
                }
            }
        }
        return result;
    }

    bool operator==(const Matrix<T>& rhs) const {
        if (_lines != rhs.getlines() || _columns != rhs.getcolumns()) {
            return false;
        }
        for (size_t i = 0; i < _columns * _lines; i++) {
            double difference = abs(static_cast<double>(_Matrixptr[i]) - static_cast<double>(rhs._Matrixptr[i]));
            if (difference > epsilon) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const Matrix<T>& rhs) const {
        return !(*this == rhs);
    }

    Matrix& operator=(const Matrix& other) {
        if (this != &other) {
            delete[] _Matrixptr;
            _Matrixptr = nullptr;

            _lines = other._lines;
            _columns = other._columns;

            if (other._Matrixptr != nullptr && _lines > 0 && _columns > 0) {
                _Matrixptr = new T[_lines * _columns];
                for (size_t i = 0; i < _lines * _columns; i++) {
                    _Matrixptr[i] = other._Matrixptr[i];
                }
            }
        }
        return *this;
    }

    size_t getlines() const {
        return _lines;
    }

    size_t getcolumns() const {
        return _columns;
    }

    T* data() { return _Matrixptr; }
    const T* data() const { return _Matrixptr; }

    T operator()(int line, int column) const {
        if (line < 0 || line >= _lines || column < 0 || column >= _columns) {
            throw std::range_error("Index out of range");
        }
        return _Matrixptr[line * _columns + column];
    }

    T& operator()(int line, int column) {
        if (line < 0 || line >= _lines || column < 0 || column >= _columns) {
            throw std::range_error("Index out of range");
        }
        return _Matrixptr[line * _columns + column];
    }

    Matrix& operator+=(const Matrix rhs) {
        if (_lines != rhs.getlines() || _columns != rhs.getcolumns()) {
            throw invalid_argument("Different size of Matrix");
        }
        for (size_t i = 0; i < _lines * _columns; i++) {
            _Matrixptr[i] += rhs._Matrixptr[i];
        }
        return *this;
    }

    Matrix operator+(const Matrix& rhs) {
        if (_lines != rhs.getlines() || _columns != rhs.getcolumns()) {
            throw invalid_argument("Different size of Matrix");
        }
        Matrix result(*this);
        result += rhs;
        return result;
    }

    Matrix& operator-=(const Matrix rhs) {
        if (_lines != rhs.getlines() || _columns != rhs.getcolumns()) {
            throw invalid_argument("Different size of Matrix");
        }
        for (size_t i = 0; i < _lines * _columns; i++) {
            _Matrixptr[i] -= rhs._Matrixptr[i];
        }
        return *this;
    }

    Matrix operator-(const Matrix& rhs) {
        if (_lines != rhs.getlines() || _columns != rhs.getcolumns()) {
            throw invalid_argument("Different size of Matrix");
        }
        Matrix result(*this);
        result -= rhs;
        return result;
    }

    Matrix operator*(T scalar) const {
        Matrix result(_lines, _columns, 0);
        for (size_t i = 0; i < _lines * _columns; i++) {
            result._Matrixptr[i] = _Matrixptr[i] * scalar;
        }
        return result;
    }

    friend Matrix operator*(T scalar, const Matrix& rhs) {
        return rhs * scalar;
    }

    Matrix operator/(T scalar) const {
        if (scalar == 0) {
            throw invalid_argument("Division by zero");
        }
        Matrix result(_lines, _columns, 0);
        for (int i = 0; i < _lines * _columns; i++) {
            result._Matrixptr[i] = _Matrixptr[i] / scalar;
        }
        return result;
    }

    T trace() const {
        T trace = 0;
        for (size_t i = 0; i < _lines; i++) {
            for (size_t j = 0; j < _columns; j++) {
                if (i == j) {
                    trace += this->operator()(i, j);
                }
            }
        }
        return trace;
    }

    Matrix operator*(const Matrix& rhs) const {
        if (_columns != rhs._lines) {
            throw invalid_argument("Uncorrect size of matrix for multiply");
        }
        Matrix<T> result(_lines, rhs._columns, 0);
        for (size_t i = 0; i < _lines; i++) {
            for (size_t j = 0; j < rhs._columns; j++) {
                for (size_t k = 0; k < _columns; k++) {
                    result(i, j) += this->operator()(i, k) * rhs(k, j);
                }
            }
        }
        return result;
    }

    ~Matrix() {
        delete[] _Matrixptr;
    }
};

template<>
inline bool Matrix<std::complex<float>>::operator==(const Matrix<std::complex<float>>& rhs) const {
    if (_lines != rhs._lines || _columns != rhs._columns) {
        return false;
    }
    for (size_t i = 0; i < _columns * _lines; i++) {
        double difference_re = abs((double)_Matrixptr[i].real() - (double)rhs._Matrixptr[i].real());
        double difference_im = abs((double)_Matrixptr[i].imag() - (double)rhs._Matrixptr[i].imag());
        if (difference_re > epsilon || difference_im > epsilon) {
            return false;
        }
    }
    return true;
}

template<>
inline bool Matrix<std::complex<double>>::operator==(const Matrix<std::complex<double>>& rhs) const {
    if (_lines != rhs._lines || _columns != rhs._columns) {
        return false;
    }
    for (size_t i = 0; i < _columns * _lines; i++) {
        double difference_re = abs((double)_Matrixptr[i].real() - (double)rhs._Matrixptr[i].real());
        double difference_im = abs((double)_Matrixptr[i].imag() - (double)rhs._Matrixptr[i].imag());
        if (difference_re > epsilon || difference_im > epsilon) {
            return false;
        }
    }
    return true;
}

template <typename T>
CudaStats<T> multiply_matrix_cuda(const Matrix<T>& a, const Matrix<T>& b,
    dim3 block_size = dim3(16, 16),
    bool use_shared_mem = true) {
    if (a.getcolumns() != b.getlines()) {
        throw invalid_argument("Matrix dimensions mismatch for multiplication");
    }

    if (a.getlines() != a.getcolumns() || b.getlines() != b.getcolumns()) {
        throw invalid_argument("Only square matrices are supported in this CUDA implementation");
    }

    size_t N = a.getlines();
    CudaStats<T> stats;
    stats.block_size = block_size;
    stats.used_shared_mem = use_shared_mem;
    stats.matrix_size = N;

    dim3 grid_size((N + block_size.x - 1) / block_size.x,
        (N + block_size.y - 1) / block_size.y);
    stats.grid_size = grid_size;

    T* d_A = nullptr, * d_B = nullptr, * d_C = nullptr;
    size_t bytes = N * N * sizeof(T);

    cudaError_t err;
    err = cudaMalloc(&d_A, bytes);
    if (err != cudaSuccess) throw runtime_error("cudaMalloc failed for d_A");
    err = cudaMalloc(&d_B, bytes);
    if (err != cudaSuccess) throw runtime_error("cudaMalloc failed for d_B");
    err = cudaMalloc(&d_C, bytes);
    if (err != cudaSuccess) throw runtime_error("cudaMalloc failed for d_C");

    err = cudaMemcpy(d_A, a.data(), bytes, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) throw runtime_error("cudaMemcpy failed for d_A");
    err = cudaMemcpy(d_B, b.data(), bytes, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) throw runtime_error("cudaMemcpy failed for d_B");

    cudaDeviceSynchronize();

    auto start = chrono::high_resolution_clock::now();

    if (use_shared_mem) {
        dim3 adjusted_block = block_size;
        if (adjusted_block.x > 32) adjusted_block.x = 32;
        if (adjusted_block.y > 32) adjusted_block.y = 32;

        matmul_kernel_shared<T> << <grid_size, adjusted_block >> > (d_A, d_B, d_C, N);
    }
    else {
        matmul_kernel_naive<T> << <grid_size, block_size >> > (d_A, d_B, d_C, N);
    }

    err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        throw runtime_error("CUDA kernel execution failed");
    }

    auto end = chrono::high_resolution_clock::now();
    stats.duration = chrono::duration_cast<chrono::microseconds>(end - start);

    Matrix<T> result(N, N, 0);
    err = cudaMemcpy(result.data(), d_C, bytes, cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) throw runtime_error("cudaMemcpy failed for result");

    stats.matrix = result;

    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    return stats;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& Matrix) {
    for (size_t i = 0; i < Matrix.getlines(); i++) {
        for (size_t j = 0; j < Matrix.getcolumns(); j++) {
            os << Matrix(i, j) << " ";
        }
        os << "\n";
    }
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const CudaStats<T>& stats) {
    os << "Matrix size: " << stats.matrix_size << "x" << stats.matrix_size << "\n";
    os << "Duration: " << stats.duration.count() << " microseconds\n";
    os << "Grid size: (" << stats.grid_size.x << "," << stats.grid_size.y << ")\n";
    os << "Block size: (" << stats.block_size.x << "," << stats.block_size.y << ")\n";
    os << "Shared memory used: " << (stats.used_shared_mem ? "yes" : "no") << "\n";
    return os;
}

#endif