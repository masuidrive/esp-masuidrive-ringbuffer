#include "ring_buffer.h"
#include <memory.h>

// メモリの書き込み関数
int _ring_buffer_mem_write(RingBuffer *buffer, const uint8_t *data, size_t size) {
  if (buffer->write_finished) {
    return RING_BUFFER_FINISHED;
  }
  if (buffer->cancelled) {
    return RING_BUFFER_CANCELED;
  }

  size_t written = 0;
  for (size_t i = 0; i < size; ++i) {
    size_t next_head = (buffer->memory_head + 1) % buffer->memory_size;
    if (next_head == buffer->memory_tail) {
      // メモリがいっぱいの場合
      return written; // 書き込んだバイト数を返す
    }
    buffer->memory_buffer[buffer->memory_head] = data[i];
    buffer->memory_head = next_head;
    written++;
  }

  return RING_BUFFER_OK;
}

// メモリの読み込み関数
int _ring_buffer_mem_read(RingBuffer *buffer, uint8_t *data, size_t size) {
  if (buffer->cancelled) {
    return RING_BUFFER_CANCELED;
  }

  size_t read_count = 0;
  while (read_count < size && buffer->memory_tail != buffer->memory_head) {
    data[read_count++] = buffer->memory_buffer[buffer->memory_tail];
    buffer->memory_tail = (buffer->memory_tail + 1) % buffer->memory_size;
  }

  return read_count;
}
