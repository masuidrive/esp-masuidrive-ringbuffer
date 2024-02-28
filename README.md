# RingBuffer Library

[![Component Registry](https://components.espressif.com/components/masuidrive/masuidrive-ringbuffer/badge.svg)](https://components.espressif.com/components/masuidrive/masuidrive-ringbuffer)

This library provides a generic FIFO ring buffer for use in the ESP-IDF environment. It features a two-tiered structure with memory and disk storage, supporting data read and write operations, along with notifications for write completion and cancellation. The library is designed to be thread-safe, utilizing FreeRTOS semaphores to safely handle concurrent access from different tasks.

## Key features:

- A FIFO ring buffer with a two-tiered structure of memory and disk storage.
- Notifications for write completion and cancellation.
- Thread-safe design using FreeRTOS semaphores.
- Initialization function that accepts pointers for the memory buffer and the `RingBuffer` structure.
- Data reading that waits until the specified number of bytes is read or until the write operation is completed.
- Cancellation handling when the file size exceeds the limit.
- Test code is also provided.

This library is useful for buffering data and asynchronous processing in IoT device development with ESP-IDF.

For more details about this library, please visit [components/masuidrive-ringbuffer](https://github.com/masuidrive/esp-masuidrive-ringbuffer/tree/main/components/masuidrive-ringbuffer).

License: Apache-2.0
Author: Yuichiro Masui <masui@masuidrive.jp>
