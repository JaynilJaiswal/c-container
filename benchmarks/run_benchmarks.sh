#!/bin/bash
set -e

echo "Building project..."
make clean && make

echo "============================================="
echo " RUNNING NANO-FAAS INDUSTRY BENCHMARKS"
echo "============================================="

PAYLOAD_MB=250 # 250 MB payload per test

echo -e "\n[1/3] Workload: Storage Deduplication (I/O Bound)"
./c-container run examples/hash_worker $PAYLOAD_MB

echo -e "\n[2/3] Workload: Telemetry Aggregation (Memory Bound)"
./c-container run examples/log_worker $PAYLOAD_MB

echo -e "\n[3/3] Workload: Tensor Mathematics (CPU Bound)"
./c-container run examples/matrix_worker $PAYLOAD_MB

echo -e "\n============================================="
echo " Benchmarks Complete."