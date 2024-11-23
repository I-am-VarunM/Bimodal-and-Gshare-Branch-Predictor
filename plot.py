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
m_values = list(range(7, 13))

results = {}

for benchmark in benchmarks:
    print(f"\n{benchmark.upper()} Misprediction Rates:")
    rates = []
    for m in m_values:
        rate = extract_rate(f'{benchmark}_m{m}.out')
        rates.append(rate)
        print(f"m = {m}: {rate}%")
    results[benchmark] = rates

# Create separate plots for each benchmark
for benchmark in benchmarks:
    plt.figure(figsize=(10, 6))
    plt.plot(m_values, results[benchmark], marker='o')
    plt.title(f'{benchmark.upper()} Benchmark: Bimodal Predictor')
    plt.xlabel('m (Number of PC bits used)')
    plt.ylabel('Misprediction Rate (%)')
    plt.grid(True)
    plt.xticks(m_values)
    if benchmark == 'jpeg':
        plt.ylim(7.5,8)
    else:
        plt.ylim(bottom=0)  # Start y-axis from 0
    plt.savefig(f'{benchmark}_bimodal_predictor_plot.png')
    plt.close()  # Close the figure to free up memory

print("\nPlots have been saved as 'gcc_bimodal_predictor_plot.png' and 'jpeg_bimodal_predictor_plot.png'")

# Print data for easy copying
print("\nData for report:")
for benchmark in benchmarks:
    print(f"{benchmark}_rates = {results[benchmark]}")