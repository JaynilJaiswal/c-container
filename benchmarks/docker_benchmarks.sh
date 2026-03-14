#!/bin/bash
set -e

echo "Building Docker Image..."
docker build -t nano-faas-docker -f Dockerfile .

echo "Building Data Generator..."
gcc -O3 benchmarks/data_generator.c -o benchmarks/data_generator

PAYLOAD_MB=250

echo -e "\n============================================="
echo " RUNNING DOCKER INDUSTRY BENCHMARKS"
echo "============================================="

echo -e "\n[1/3] Docker Workload: Storage Deduplication"
time ./benchmarks/data_generator hash $PAYLOAD_MB | docker run -i --rm nano-faas-docker /workers/hash_worker

echo -e "\n[2/3] Docker Workload: Telemetry Aggregation"
time ./benchmarks/data_generator log $PAYLOAD_MB | docker run -i --rm nano-faas-docker /workers/log_worker

echo -e "\n[3/3] Docker Workload: Tensor Mathematics"
time ./benchmarks/data_generator matrix $PAYLOAD_MB | docker run -i --rm nano-faas-docker /workers/matrix_worker

echo -e "\n============================================="
echo " Docker Benchmarks Complete."