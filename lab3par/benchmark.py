import subprocess
import csv

sizes = [200, 400, 800, 1200, 1600, 2000]
procs_list = [1, 2, 4, 8, 16]

results_file = "mpi_full_results.csv"

with open(results_file, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["N", "Processes", "Time_sec"])

    for N in sizes:

        print(f"\n=== SIZE {N} ===")

        subprocess.run(f"generate.exe {N} inputA.txt inputB.txt", shell=True)

        for p in procs_list:

            print(f"Running: N={N}, P={p}")

            result = subprocess.run(
                f"mpiexec -n {p} main.exe inputA.txt inputB.txt result.txt",
                shell=True,
                capture_output=True,
                text=True
            )

            print(result.stdout)

            try:
                last = result.stdout.strip().split("\n")[-1]
                time_sec = float(last.split("TIME")[1].split("sec")[0])
            except:
                time_sec = -1

            writer.writerow([N, p, time_sec])

print("\nDONE -> mpi_full_results.csv")
