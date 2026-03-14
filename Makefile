CC = gcc
CFLAGS = -Wall -O3 -D_GNU_SOURCE
TARGET = c-container
SRC = src/main.c

WORKERS = examples/hash_worker examples/log_worker examples/matrix_worker

all: $(TARGET) $(WORKERS)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

examples/%: examples/%.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) $(WORKERS)

.PHONY: all clean