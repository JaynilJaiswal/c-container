# Nano-FaaS vs. Docker: Empirical Benchmarks

This document details the performance differences between Nano-FaaS (`c-container`) and standard Docker (`alpine:latest` via `containerd`/`runc`). 

Both engines were tasked with executing identical compiled C binaries. For each workload, a 250 MB payload was dynamically generated and streamed directly into the isolated worker process. 

## 📊 Results Summary

| Workload | Hardware Bottleneck | Docker (Total Time) | Nano-FaaS (Total Time) | Nano-FaaS Speedup |
| :--- | :--- | :--- | :--- | :--- |
| **Storage Deduplication** (FNV-1a) | I/O & IPC Bandwidth | 0.508 sec | **0.414 sec** | **1.22x Faster** |
| **Telemetry Aggregation** (Nginx) | Memory / String Parsing | 0.659 sec | **0.380 sec** | **1.73x Faster** |
| **Tensor Mathematics** (Dot Product) | Pure CPU Compute | 0.342 sec | **0.153 sec** | **2.23x Faster** |

*(Note: "Total Time" includes the full lifecycle: namespace/container allocation, payload streaming, computation, and process teardown).*

## 🔬 Architectural Analysis

### Why is Nano-FaaS over 2x faster for Tensor Math?
For pure compute workloads like LLM tensor math, standard containers introduce severe "cold start" latency. `docker run` must communicate with the Docker daemon over a Unix socket, instruct `containerd` to provision the environment, and use `runc` to mount virtual filesystems (OverlayFS) and initialize `cgroups`. 

Nano-FaaS bypasses this completely. By directly invoking the `clone()` syscall with `CLONE_NEWPID` and `CLONE_NEWUTS`, it achieves process isolation in less than 2 milliseconds. 

### Why the smaller margin on Storage Deduplication?
The Storage Deduplication workload is bound by I/O and IPC throughput. While Nano-FaaS still wins due to its rapid startup time, both engines are ultimately constrained by how fast the Linux kernel can move 250 MB of data through a pipe from the host dispatcher to the isolated worker.

### Conclusion
For long-running web servers, standard Docker containers remain the industry standard. However, for short-lived, high-frequency tasks—such as Serverless functions, log parsers, or discrete ML data transformations—dropping down to raw Linux kernel primitives (Namespaces + IPC Pipes) yields a massive reduction in latency and computational overhead.