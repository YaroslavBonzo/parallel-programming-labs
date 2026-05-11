#include <mpi.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

vector<double> readMatrix(const string& filename, int& N) {
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Ошибка открытия файла: " << filename << endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    file >> N;

    vector<double> mat(N * N);

    for (int i = 0; i < N * N; i++)
        file >> mat[i];

    return mat;
}

void writeMatrix(const string& filename, const vector<double>& mat, int N) {
    ofstream file(filename);

    file << N << "\n";

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            file << mat[i * N + j] << " ";
        }
        file << "\n";
    }
}

int main(int argc, char* argv[]) {

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 4) {
        if (rank == 0)
            cout << "Usage: mpiexec -n P main.exe inputA.txt inputB.txt output.txt\n";
        MPI_Finalize();
        return 0;
    }

    string fileA = argv[1];
    string fileB = argv[2];
    string fileOut = argv[3];

    int N;

    vector<double> A, B, C;

    if (rank == 0) {

        A = readMatrix(fileA, N);
        B = readMatrix(fileB, N);
        C.resize(N * N);
    }

    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int base = N / size;
    int rem = N % size;

    vector<int> sendcounts(size);
    vector<int> displs(size);

    int offset = 0;

    for (int i = 0; i < size; i++) {
        int rows = base + (i < rem ? 1 : 0);
        sendcounts[i] = rows * N;
        displs[i] = offset;
        offset += rows * N;
    }

    int local_size = sendcounts[rank];
    int local_rows = local_size / N;

    vector<double> localA(local_size);
    vector<double> localC(local_size, 0.0);

    MPI_Scatterv(
        rank == 0 ? A.data() : nullptr,
        sendcounts.data(),
        displs.data(),
        MPI_DOUBLE,
        localA.data(),
        local_size,
        MPI_DOUBLE,
        0,
        MPI_COMM_WORLD
    );

    if (rank != 0)
        B.resize(N * N);

    MPI_Bcast(B.data(), N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    double t1 = MPI_Wtime();

    for (int i = 0; i < local_rows; i++) {
        for (int j = 0; j < N; j++) {

            double sum = 0.0;

            for (int k = 0; k < N; k++) {
                sum += localA[i * N + k] * B[k * N + j];
            }

            localC[i * N + j] = sum;
        }
    }

    MPI_Gatherv(
        localC.data(),
        local_size,
        MPI_DOUBLE,
        rank == 0 ? C.data() : nullptr,
        sendcounts.data(),
        displs.data(),
        MPI_DOUBLE,
        0,
        MPI_COMM_WORLD
    );

    MPI_Barrier(MPI_COMM_WORLD);
    double t2 = MPI_Wtime();

    if (rank == 0) {

        cout << "SIZE " << N
            << " | PROCS " << size
            << " | TIME " << (t2 - t1)
            << " sec\n";

        writeMatrix(fileOut, C, N);
    }

    MPI_Finalize();
    return 0;
}
