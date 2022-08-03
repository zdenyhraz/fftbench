import csv
import numpy as np
import matplotlib.pyplot as plt


class BenchmarkEntry:
  def __init__(self, size, time):
    self.sizes = np.array([size])
    self.times = np.array([time])


def find_nth(haystack, needle, n):
  start = haystack.find(needle)
  while start >= 0 and n > 1:
    start = haystack.find(needle, start+len(needle))
    n -= 1
  return start


time_unit_input = 1e-6
time_unit = 1e-3
entries = {}
minsize = np.inf
maxsize = 0

with open('build/fftbench.csv', newline='') as csvfile:
  reader = csv.reader(csvfile, delimiter=',')
  for row in reader:
    string = ', '.join(row)
    if " | " in string:
      print("Processing", string)
      size = int(string[0:string.find(" | ")].strip())
      name = string[string.find(" | ")+2:string.find(",")].strip()
      time = float(string[find_nth(string, ",", 2)+1:find_nth(string, ",", 3)].strip())/(time_unit/1e-6)  # [ms]
      minsize = min(minsize, size)
      maxsize = max(maxsize, size)

      if name in entries:
        entries[name].sizes = np.append(entries[name].sizes, size)
        entries[name].times = np.append(entries[name].times, time)
      else:
        entries[name] = BenchmarkEntry(size, time)


xticksvalues = []
xtickslabels = []
for exp in range(int(np.log2(minsize)), int(np.log2(maxsize))+1):
  xticksvalues.append(np.power(2, exp))
  xtickslabels.append("$2^{" + "{}".format(int(exp)) + "}$")

plt.figure(figsize=(10, 10))
plt.subplot(2, 1, 1)
for name, entry in entries.items():
  plt.plot(entry.sizes, entry.times, "*-", label=name, linewidth=2)
plt.xscale("log")
plt.xlabel("FFT size")
plt.ylabel("time [ms]")
plt.xticks(xticksvalues, xtickslabels)
plt.legend()

plt.subplot(2, 1, 2)
for name, entry in entries.items():
  plt.plot(entry.sizes, entry.times, "*-", label=name, linewidth=2)
plt.xscale("log")
plt.yscale("log")
plt.xlabel("FFT size")
plt.ylabel("time [ms]")
plt.xticks(xticksvalues, xtickslabels)
plt.legend()

plt.tight_layout()
plt.show()
