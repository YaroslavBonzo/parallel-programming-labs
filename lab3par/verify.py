import numpy as np
import sys

def read_matrix(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()

    N = int(lines[0].strip())
    matrix = []

    for i in range(1, N + 1):
        row = list(map(float, lines[i].split()))
        matrix.append(row)

    return np.array(matrix), N


if __name__ == "__main__":

    if len(sys.argv) != 4:
        print("Использование: python verify.py inputA.txt inputB.txt result.txt")
        sys.exit(1)

    fileA = sys.argv[1]
    fileB = sys.argv[2]
    fileC = sys.argv[3]

    A, N1 = read_matrix(fileA)
    B, N2 = read_matrix(fileB)
    C_cpp, N3 = read_matrix(fileC)

    if N1 != N2 or N1 != N3:
        print("Ошибка: размеры матриц не совпадают!")
        sys.exit(1)

    C_numpy = np.dot(A, B)

    if np.allclose(C_numpy, C_cpp):
        print("✅ Верификация пройдена: результаты совпадают.")
    else:
        print("❌ Ошибка: результаты НЕ совпадают.")
        diff = np.abs(C_numpy - C_cpp)
        print("Максимальное отклонение:", np.max(diff))