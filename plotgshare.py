import re
import matplotlib.pyplot as plt

def extract_rate(filename):
    with open(filename, 'r') as f:
        content = f.read()
        match = re.search(r'misprediction rate:\s+([\d.]+)%', content)
        if match:
            return float(match.group(1))
    return None

benchmarks = ['gcc', 'jpeg']
m_values = range(7, 13)
n_values = range(2, 13, 2)

results = {benchmark: {n: [] for n in n_values} for benchmark in benchmarks}

for benchmark in benchmarks:
    for m in m_values:
        for n in n_values:
            if n <= m:
                rate = extract_rate(f'{benchmark}_m{m}_n{n}.out')
                if rate is not None:
                    results[benchmark][n].append((m, rate))

# Create plots
for benchmark in benchmarks:
    plt.figure(figsize=(12, 8))
    for n in n_values:
        data = results[benchmark][n]
        if data:
            m_vals, rates = zip(*data)
            plt.plot(m_vals, rates, marker='o', label=f'n={n}')
    
    plt.title(f'{benchmark.upper()} Benchmark: Gshare Predictor')
    plt.xlabel('m (PC bits used)')
    plt.ylabel('Misprediction Rate (%)')
    plt.legend(title='n (History bits)', loc='upper right')
    plt.grid(True)
    plt.xticks(m_values)
    plt.savefig(f'{benchmark}_gshare_predictor_plot.png')
    plt.close()

print("Plots have been saved as 'gcc_gshare_predictor_plot.png' and 'jpeg_gshare_predictor_plot.png'")

# Print data for easy copying
print("\nData for report:")
for benchmark in benchmarks:
    print(f"\n{benchmark.upper()} Misprediction Rates:")
    for n in n_values:
        data = results[benchmark][n]
        if data:
            print(f"n = {n}: {[rate for _, rate in data]}")