#!/bin/bash

benchmarks=("gcc" "jpeg")

for benchmark in "${benchmarks[@]}"; do
    for m in {7..12}; do
        for n in $(seq 2 2 $m); do
            echo "Running: ./bpsim gshare $m $n ${benchmark}_trace.txt"
            ./bpsim gshare $m $n ${benchmark}_trace.txt > ${benchmark}_m${m}_n${n}.out
        done
    done
done

echo "All simulations completed."