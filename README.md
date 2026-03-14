
# Nano-FaaS: Sub-Millisecond Serverless Execution Engine

![Language](https://img.shields.io/badge/Language-C99-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Linux-lightgrey.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

*A lightweight, C-based container runtime designed for extreme high-throughput data processing and zero-overhead cold starts.*

## 📖 Overview

**The Problem:** Modern container runtimes (like `containerd` or Docker) are excellent for packaging complex dependency trees, but they introduce heavy overhead for transient, short-lived tasks. When building high-frequency data pipelines, specialized machine learning infrastructure, or Serverless (FaaS) platforms, the 500ms+ cold start and heavy memory footprint of a standard container are unacceptable.

**The Solution:** Nano-FaaS drops down to raw Linux kernel primitives. By utilizing the `clone()` syscall with specific namespaces (`CLONE_NEWPID`, `CLONE_NEWUTS`) and synchronous IPC pipes, this engine achieves the fault-isolation of a container with the sub-millisecond startup latency of a lightweight thread. 

It is designed to act as a highly efficient **dispatcher**, streaming payloads in-memory to isolated worker processes that compute and terminate in milliseconds.

## ⚙️ Systems Architecture

Unlike standard OCI containers that mount complex OverlayFS layers and virtual network bridges, Nano-FaaS optimizes for pure computational throughput:

1. **The Dispatcher (Host):** Allocates a minimal isolated stack (`malloc`) and creates a unidirectional Inter-Process Communication (IPC) pipe.
2. **Process Isolation:** The worker is spawned via `clone()`. `CLONE_NEWPID` ensures the worker is isolated in its own process tree (PID 1), protecting the host from fork-bombs or rogue threads. `CLONE_NEWUTS` isolates the hostname.
3. **Synchronous Data Streaming:** The host maps the read-end of the IPC pipe directly to the worker's `STDIN_FILENO`. Data is blasted entirely in-memory from the host to the isolated worker, completely bypassing disk I/O.
4. **Reaping:** The worker processes the data stream, outputs the result, and exits. The host cleanly reaps the process via `waitpid()`.

## 📊 Industry Benchmarks

The repository includes three distinct industry workloads to benchmark different hardware bottlenecks: **Storage Deduplication** (I/O Bound), **Telemetry Aggregation** (Memory Bound), and **Tensor Mathematics** (CPU Bound).

*Benchmarks generated using identical compiled C binaries processing 250 MB payloads.*

| Workload | Docker Container (`alpine`) | Nano-FaaS (This Engine) | Performance Gain |
| :--- | :--- | :--- | :--- |
| **Storage Deduplication** | 0.508 sec | **0.414 sec** | 22% Faster |
| **Telemetry Aggregation** | 0.659 sec | **0.380 sec** | 73% Faster |
| **Tensor Mathematics** | 0.342 sec | **0.153 sec** | **123% Faster** |

**The Nano-FaaS Advantage:** Standard runtimes require ~300ms+ just for the "cold start" before computation begins (daemon communication, OverlayFS mounting, virtual bridging). By directly utilizing `clone()` and synchronous Unix pipes, Nano-FaaS achieves process isolation and execution in < 2ms, yielding over a 2x speedup in compute-bound Serverless tasks.

## 🚀 Getting Started

### Prerequisites
* A Linux host system (requires native Linux syscalls and capabilities).
* GCC and Make.
* Execution requires `sudo` or `CAP_SYS_ADMIN` capabilities to create new kernel namespaces.

### Building
Compile the host dispatcher and all specialized worker binaries:
```bash
make clean && make

```

### Running the Benchmarks

Run the automated benchmarking script to simulate processing 250MB payloads across all three workloads:

```bash
sudo ./benchmarks/run_benchmarks.sh

```

### Manual Execution

You can manually dispatch a worker by providing the worker binary path and the payload size (in MB):

```bash
# Run the Tensor Math worker with a 100MB serialized float payload
sudo ./c-container run examples/matrix_worker 100

```

## 📂 Repository Structure

```text
├── Makefile                   # Build system for dispatcher and workers
├── src/
│   └── main.c                 # The Core Host Dispatcher / Namespace logic
├── examples/
│   ├── hash_worker.c          # Workload 1: FNV-1a Storage Deduplication
│   ├── log_worker.c           # Workload 2: Nginx Telemetry Aggregation
│   └── matrix_worker.c        # Workload 3: LLM Tensor Math Processing
└── benchmarks/
    └── run_benchmarks.sh      # Automated benchmarking suite

```

## 🔮 Future Work

* **Memory Limits:** Integrate `cgroups` (v2) to allow the host to enforce strict RAM and CPU quotas on individual worker processes.
* **Seccomp BPF:** Attach a seccomp filter to the worker process to restrict the available syscalls, further reducing the attack surface of the isolated function.
