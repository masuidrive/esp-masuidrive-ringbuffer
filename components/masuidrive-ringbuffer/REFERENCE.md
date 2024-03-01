RingBuffer Library

This library provides a generic FIFO ring buffer that operates in the ESP-IDF environment. It features a two-tiered structure with memory and disk storage, supports data read and write operations, and includes notification functions for write completion and cancellation. The design is thread-safe using FreeRTOS semaphores, allowing for safe concurrent access from different tasks.

Key features include:

- A FIFO ring buffer with a two-tiered structure of memory and disk storage.
- Notification functions for write completion and cancellation.
- Thread-safe design using FreeRTOS semaphores.
- An initialization function that accepts pointers to the memory buffer and the `RingBuffer` structure.
- Data read operation that waits for the specified number of bytes to be read or until it finishes.
- Cancellation process when the file size is exceeded.

This library is useful for buffering data and asynchronous processing in IoT device development in the ESP-IDF environment.

## Function Documentation

### `void ring_buffer_init(RingBuffer *buffer, uint8_t *memory, size_t memory_size, const char *file_name, size_t file_size)`

Initializes the ring buffer. It sets up the memory buffer and file buffer sizes for initialization.

- `buffer`: Pointer to the `RingBuffer` structure.
- `memory`: Pointer to the memory buffer.
- `memory_size`: Size of the memory buffer.
- `file_name`: Name of the file to be used.
- `file_size`: Size of the file buffer.

### `int ring_buffer_write(RingBuffer *buffer, const uint8_t *data, size_t size)`

Writes data to the ring buffer. If the memory buffer is full, it writes to the file buffer.

- `buffer`: Pointer to the `RingBuffer` structure.
- `data`: Pointer to the data to be written.
- `size`: Size of the data to be written.

The return value is a constant indicating the result of the write operation (`RING_BUFFER_OK`, `RING_BUFFER_OVERFLOW`, `RING_BUFFER_FINISHED`, `RING_BUFFER_CANCELED`).

### `int ring_buffer_read(RingBuffer *buffer, uint8_t *data, size_t size, TickType_t xTicksToWait)`

Reads data from the ring buffer. It waits for the specified number of bytes to be read or until the specified time.

- `buffer`: Pointer to the `RingBuffer` structure.
- `data`: Pointer to the buffer where the read data will be stored.
- `size`: Number of bytes to read.
- `xTicksToWait`: Time to wait (in FreeRTOS ticks).

The return value is the actual number of bytes read. If the operation is canceled, it returns `RING_BUFFER_CANCELED`.

### `void ring_buffer_finish_write(RingBuffer *buffer)`

Finishes writing to the ring buffer. After finishing, the `ring_buffer_write` function will return `RING_BUFFER_FINISHED`.

- `buffer`: Pointer to the `RingBuffer` structure.

### `void ring_buffer_cancel(RingBuffer *buffer)`

Cancels the operation of the ring buffer. After cancellation, the `ring_buffer_write` and `ring_buffer_read` functions will return `RING_BUFFER_CANCELED`.

- `buffer`: Pointer to the `RingBuffer` structure.

### `void ring_buffer_free(RingBuffer *buffer)`

Frees the ring buffer. It releases the resources used.

- `buffer`: Pointer to the `RingBuffer` structure.
