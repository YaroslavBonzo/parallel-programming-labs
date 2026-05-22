#include <iostream>
#include "Matrix.h"
#include <fstream>
#include <chrono>
#include <vector>
#include <iomanip>

using namespace std;

int main() {
    cout << "Lab4 (CUDA): Matrix Multiplication with CUDA\n";
    cout << "Student: Slipenchuk Nikita 6212-100503D\n";
    cout << "GPU: NVIDIA GeForce RTX 3050 Laptop GPU (4 GB)\n\n";

    vector<int> sizes = { 200, 400, 800, 1200, 1600, 2000 };

    vector<dim3> block_sizes = {
        dim3(8, 8),
        dim3(16, 16),
        dim3(32, 32)
    };

    ofstream results("cuda_results.txt");
    results << "CUDA Matrix Multiplication Results\n";
    results << "==================================\n";
    results << "Student: Slipenchuk Nikita\n";
    results << "GPU: NVIDIA GeForce RTX 3050 Laptop GPU\n\n";

    results << left << setw(12) << "Size"
        << setw(20) << "Block config"
        << setw(14) << "Time (us)" << "\n";
    results << string(46, '-') << "\n";

    for (int n : sizes) {
        cout << "\nTesting matrix size: " << n << "x" << n << "\n";

        Matrix<int> A(n, n, 1, 10);
        Matrix<int> B(n, n, 1, 10);

        for (dim3 block : block_sizes) {
            cout << "  Block(" << block.x << "," << block.y << ")... ";

            try {
                auto stats = multiply_matrix_cuda(A, B, block, true);
                cout << stats.duration.count() << " us\n";

                results << left << setw(12) << n
                    << setw(20) << (to_string(block.x) + "x" + to_string(block.y))
                    << setw(14) << stats.duration.count() << "\n";

            }
            catch (const exception& e) {
                cout << "ERROR: " << e.what() << "\n";
                results << left << setw(12) << n
                    << setw(20) << (to_string(block.x) + "x" + to_string(block.y))
                    << setw(14) << "ERROR" << "\n";
            }
        }
        results << string(46, '-') << "\n";
    }

    results.close();
    cout << "\nComplete! Results saved to cuda_results.txt\n";

    return 0;
}