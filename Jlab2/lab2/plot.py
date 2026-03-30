import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results.csv")

df["Time"] = df["Time"].astype(float)

plt.figure()

for t in sorted(df["Threads"].unique()):
    subset = df[df["Threads"] == t]
    plt.plot(subset["N"], subset["Time"], marker='o', label=f"{t} threads")

plt.xlabel("Matrix size (N)")
plt.ylabel("Time (seconds)")
plt.title("Execution time vs matrix size")
plt.legend()
plt.grid()

plt.savefig("time_vs_size.png")
plt.show()

plt.figure()

for n in sorted(df["N"].unique()):
    subset = df[df["N"] == n]
    plt.plot(subset["Threads"], subset["Time"], marker='o', label=f"N={n}")

plt.xlabel("Number of threads")
plt.ylabel("Time (seconds)")
plt.title("Execution time vs threads")
plt.legend()
plt.grid()

plt.savefig("time_vs_threads.png")
plt.show()