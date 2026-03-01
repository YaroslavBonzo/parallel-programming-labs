import numpy as np
import random
import os
import argparse
import sys


def generate_matrices(size, output_dir='data'):
    """
    Генерация двух случайных квадратных матриц заданного размера
    """
    os.makedirs(output_dir, exist_ok=True)
    random.seed(52)

    # Генерация матриц со случайными целыми числами
    matrix_a = [[random.randint(-100, 100) for _ in range(size)] for _ in range(size)]
    matrix_b = [[random.randint(-100, 100) for _ in range(size)] for _ in range(size)]

    # Сохранение первой матрицы
    with open(f'{output_dir}/matrix_a.txt', 'w') as f:
        f.write(str(size) + '\n')
        for row in matrix_a:
            f.write(' '.join(map(str, row)) + '\n')

    # Сохранение второй матрицы
    with open(f'{output_dir}/matrix_b.txt', 'w') as f:
        f.write(str(size) + '\n')
        for row in matrix_b:
            f.write(' '.join(map(str, row)) + '\n')

    print(f'Матрицы {size}x{size} созданы в директории {output_dir}/')
    return matrix_a, matrix_b


def read_matrix(filename):
    """
    Чтение матрицы из файла
    """
    try:
        with open(filename, 'r') as f:
            lines = f.readlines()

        size = int(lines[0].strip())
        matrix = []

        for i in range(1, size + 1):
            row = list(map(int, lines[i].strip().split()))
            matrix.append(row)

        return np.array(matrix), size

    except FileNotFoundError:
        print(f"Ошибка: Файл {filename} не найден")
        sys.exit(1)
    except Exception as e:
        print(f"Ошибка при чтении {filename}: {e}")
        sys.exit(1)


def verify_results():
    """
    Верификация результатов умножения матриц
    """
    print("\n" + "=" * 50)
    print("ВЕРИФИКАЦИЯ РЕЗУЛЬТАТОВ")
    print("=" * 50)

    # Чтение исходных матриц
    matrix_a, size_a = read_matrix('data/matrix_a.txt')
    matrix_b, size_b = read_matrix('data/matrix_b.txt')

    # Чтение результата
    matrix_result, size_result = read_matrix('data/result.txt')

    # Проверка размерностей
    if size_a != size_b:
        print(f"Ошибка: Размеры матриц не совпадают ({size_a} != {size_b})")
        return False

    if size_result != size_a:
        print(f"Ошибка: Размер результата ({size_result}) не совпадает с исходными ({size_a})")
        return False

    # Вычисление через NumPy
    matrix_correct = np.dot(matrix_a, matrix_b)

    # Сравнение
    if np.array_equal(matrix_result, matrix_correct):
        print("\n✓ РЕЗУЛЬТАТ ВЕРЕН: Матрицы совпадают")
        return True
    else:
        print("\n✗ ОШИБКА: Результат не совпадает")
        diff = np.abs(matrix_result - matrix_correct)
        print(f"Максимальная разница: {np.max(diff)}")
        return False


def main():
    parser = argparse.ArgumentParser(description='Генерация и верификация матриц')
    subparsers = parser.add_subparsers(dest='command', help='Команды')

    # Генерация
    gen_parser = subparsers.add_parser('generate', help='Сгенерировать матрицы')
    gen_parser.add_argument('size', type=int, help='Размер матриц')

    # Верификация
    subparsers.add_parser('verify', help='Проверить результат')

    args = parser.parse_args()

    if args.command == 'generate':
        generate_matrices(args.size)
    elif args.command == 'verify':
        success = verify_results()
        sys.exit(0 if success else 1)
    else:
        parser.print_help()


if __name__ == "__main__":
    main()