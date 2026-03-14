FROM alpine:latest
RUN apk add --no-cache gcc musl-dev

WORKDIR /workers
COPY examples/ ./examples/

# Compile the workers inside the container
RUN gcc -O3 examples/hash_worker.c -o hash_worker && \
    gcc -O3 examples/log_worker.c -o log_worker && \
    gcc -O3 examples/matrix_worker.c -o matrix_worker

# The entrypoint will be overridden by the benchmark script
CMD ["/bin/sh"]