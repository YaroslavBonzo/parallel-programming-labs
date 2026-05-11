#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <string>

using namespace std;

vector<vector<int>> generateMatrix(int N) {
    vector<vector<int>> matrix(N, vector<int>(N));

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, 9);

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            matrix[i][j] = dist(gen);

    return matrix;
}

void writeMatrix(const string& filename, const vector<vector<int>>& matrix) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Ошибка записи в файл: " << filename << endl;
        exit(1);
    }

    int N = matrix.size();
    file << N << endl;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            file << matrix[i][j] << " ";
        file << endl;
    }

    file.close();
}

int main(int argc, char* argv[]) {

    if (argc != 4) {
        cout << "Использование: ./generate N fileA.txt fileB.txt" << endl;
        return 1;
    }

    int N = stoi(argv[1]);
    string fileA = argv[2];
    string fileB = argv[3];

    vector<vector<int>> A = generateMatrix(N);
    vector<vector<int>> B = generateMatrix(N);

    writeMatrix(fileA, A);
    writeMatrix(fileB, B);

    cout << "Сгенерированы матрицы размера "
        << N << " x " << N << endl;

    return 0;
}