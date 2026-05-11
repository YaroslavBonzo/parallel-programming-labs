import csv
import matplotlib.pyplot as plt

data = {}

with open("mpi_full_results.csv", "r") as f:
    reader = csv.reader(f)
    next(reader)

    for row in reader:
        N = int(row[0])
        P = int(row[1])
        T = float(row[2])

        if N not in data:
            data[N] = {"p": [], "t": []}

        data[N]["p"].append(P)
        data[N]["t"].append(T)

plt.figure()

for N in sorted(data.keys()):
    plt.plot(data[N]["p"], data[N]["t"], marker="o", label=f"{N}x{N}")

plt.xlabel("Number of processes")
plt.ylabel("Time (seconds)")
plt.title("MPI Matrix Multiplication Performance")

plt.grid(True)
plt.legend()
plt.show()
