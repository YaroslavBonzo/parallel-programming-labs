import numpy as np
import sys

def read_matrix(inputfile: str,outputfile:str) -> np.array:
    with open(inputfile, "r") as f:
        n = int(f.readline().strip())
        matrix1 = []
        for _ in range(n):
            row = list(map(int, f.readline().strip().split()))
            matrix1.append(row)
        matrix2 = []
        for _ in range(n):
            row = list(map(int, f.readline().strip().split()))
            matrix2.append(row)
    with open(outputfile, "r") as f:
        matrix3 = []
        for _ in range(n):
            row = list(map(int, f.readline().strip().split()))
            matrix3.append(row)
    return (
        np.array(matrix1, dtype=np.int64),
        np.array(matrix2, dtype=np.int64),
        np.array(matrix3, dtype=np.int64),
    )

def main():
    matrix1,matrrix2,matrix3 = read_matrix("input.txt","output.txt")
    buf = np.dot(matrix1,matrrix2)
    is_equal = np.array_equal(buf, matrix3)
    if is_equal == True:
        sys.exit(1)
    else:
        sys.exit(0)


if __name__ == "__main__":
    main()