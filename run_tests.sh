#!/bin/bash

SIZES=(1000 5000 10000 50000 100000)
THREADS=(1 2 4 8)
REPS=3

echo "size,type,threads,time,overhead" > results.csv

for size in "${SIZES[@]}"; do
    # Serial
    total_time=0
    for i in $(seq 1 $REPS); do
        time=$(./odd_even_serial.exe $size | awk -F': ' '/Tempo de execução serial/{print $2}')
        total_time=$(echo "$total_time + $time" | bc)
    done
    avg_time=$(echo "scale=6; $total_time / $REPS" | bc)
    echo "$size,serial,1,$avg_time,0" >> results.csv

    # OpenMP
    for t in "${THREADS[@]}"; do
        total_time=0
        for i in $(seq 1 $REPS); do
            time=$(./odd_even_openmp.exe $size $t | awk -F': ' '/Tempo de execução OpenMP/{print $2}')
            total_time=$(echo "$total_time + $time" | bc)
        done
        avg_time=$(echo "scale=6; $total_time / $REPS" | bc)
        echo "$size,openmp,$t,$avg_time,0" >> results.csv
    done

    # MPI
    for p in "${THREADS[@]}"; do
        total_time=0
        total_overhead=0
        for i in $(seq 1 $REPS); do
            output=$("$MPI_BIN/mpiexec" -n $p ./odd_even_mpi.exe $size)
            time=$(echo "$output" | awk -F': ' '/Tempo Total \(max\)/{print $2}')
            overhead=$(echo "$output" | awk -F'[ %]' '/Overhead de Comunicação/{print $1}')
            total_time=$(echo "$total_time + $time" | bc)
            total_overhead=$(echo "$total_overhead + $overhead" | bc)
        done
        avg_time=$(echo "scale=6; $total_time / $REPS" | bc)
        avg_overhead=$(echo "scale=2; $total_overhead / $REPS" | bc)
        echo "$size,mpi,$p,$avg_time,$avg_overhead" >> results.csv
    done
done