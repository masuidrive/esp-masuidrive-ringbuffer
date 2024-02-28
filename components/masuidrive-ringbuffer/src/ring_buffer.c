#include "ring_buffer.h"

#include <freertos/task.h>

int _ring_buffer_mem_write(RingBuffer *buffer, const uint8_t *data, size_t size);
int _ring_buffer_mem_read(RingBuffer *buffer, uint8_t *data, size_t size);
int _ring_buffer_file_write(RingBuffer *buffer, const uint8_t *data, size_t size);
int _ring_buffer_file_read(RingBuffer *buffer, uint8_t *data, size_t size);
void _ring_buffer_create_file(FILE *file, size_t file_size);

// 初期化関数
void ring_buffer_init(RingBuffer *buffer, uint8_t *memory, size_t memory_size, const char *file_name,
                      size_t file_size) {
  buffer->memory_buffer = memory;
  buffer->memory_size = memory_size;
  buffer->memory_head = 0;
  buffer->memory_tail = 0;
  buffer->file = fopen(file_name, "w+b");
  buffer->file_size = file_size;
  buffer->file_head = 0;
  buffer->file_tail = 0;
  buffer->write_finished = false;
  buffer->cancelled = false;
  buffer->mutex = xSemaphoreCreateMutex();
}

// データの書き込み関数
int ring_buffer_write(RingBuffer *buffer, const uint8_t *data, size_t size) {
  xSemaphoreTake(buffer->mutex, portMAX_DELAY);

  if (buffer->cancelled) {
    xSemaphoreGive(buffer->mutex);
    return RING_BUFFER_CANCELED;
  }

  if (buffer->write_finished) {
    xSemaphoreGive(buffer->mutex);
    return RING_BUFFER_FINISHED;
  }

  int result = _ring_buffer_mem_write(buffer, data, size);
  if (result == RING_BUFFER_OK) {
    xSemaphoreGive(buffer->mutex);
    return RING_BUFFER_OK;
  }

  // メモリオーバーフロー時、ファイルに書き込む
  result = _ring_buffer_file_write(buffer, data + result, size - result);
  if (result == RING_BUFFER_OK) {
    xSemaphoreGive(buffer->mutex);
    return RING_BUFFER_OK;
  }

  xSemaphoreGive(buffer->mutex);
  return RING_BUFFER_OVERFLOW; // 両方オーバーフロー
}

// データの読み込み関数
int ring_buffer_read(RingBuffer *buffer, uint8_t *data, size_t size, TickType_t xTicksToWait) {
  xSemaphoreTake(buffer->mutex, portMAX_DELAY);

  if (buffer->cancelled) {
    xSemaphoreGive(buffer->mutex);
    return RING_BUFFER_CANCELED;
  }

  size_t read_count = 0;
  if (size <= buffer->memory_size) {
    // メモリから読み込む
    read_count = _ring_buffer_mem_read(buffer, data, size);
  } else {
    // メモリから読み込んで、足りなければファイルから読み込む
    read_count = _ring_buffer_mem_read(buffer, data, buffer->memory_size);
    if (read_count < buffer->memory_size) {
      xSemaphoreGive(buffer->mutex);
      return read_count;
    }
    read_count += _ring_buffer_file_read(buffer, data + buffer->memory_size, size - buffer->memory_size);
  }

  // メモリに空きがあり、ファイルにデータがある場合、ファイルからメモリに移動
  while (buffer->memory_head != buffer->memory_tail && buffer->file_tail != buffer->file_head) {
    uint8_t temp;
    if (_ring_buffer_file_read(buffer, &temp, 1) > 0) {
      _ring_buffer_mem_write(buffer, &temp, 1);
    }
  }

  xSemaphoreGive(buffer->mutex);
  return read_count;
}

// 書き込み終了関数
void ring_buffer_finish_write(RingBuffer *buffer) {
  xSemaphoreTake(buffer->mutex, portMAX_DELAY);
  buffer->write_finished = true;
  xSemaphoreGive(buffer->mutex);
}

// キャンセル関数
void ring_buffer_cancel(RingBuffer *buffer) {
  xSemaphoreTake(buffer->mutex, portMAX_DELAY);
  buffer->cancelled = true;
  xSemaphoreGive(buffer->mutex);
}

// 解放関数
void ring_buffer_free(RingBuffer *buffer) {
  fclose(buffer->file);
  vSemaphoreDelete(buffer->mutex);
}
