#include "ringbuffer.h"

#include <freertos/task.h>
#include <memory.h>
#include <stdio.h>

// 初期化関数
void ring_buffer_init(RingBuffer *buffer, uint8_t *memory, size_t memory_size,
                      const char *file_name, size_t file_max_size) {
  buffer->memory_buffer = memory;
  buffer->memory_size = memory_size;
  buffer->memory_head = 0;
  buffer->memory_tail = 0;
  buffer->file = fopen(file_name, "w+b");
  buffer->file_name = strdup(file_name);
  buffer->file_size = 0;
  buffer->file_max_size = file_max_size;
  buffer->write_finished = false;
  buffer->cancelled = false;
  buffer->mutex = xSemaphoreCreateMutex();
}

// データの書き込み関数
int ring_buffer_write(RingBuffer *buffer, const uint8_t *data, size_t size) {
  xSemaphoreTake(buffer->mutex, portMAX_DELAY);
  if (buffer->write_finished || buffer->cancelled) {
    xSemaphoreGive(buffer->mutex);
    return -1;  // 書き込み終了済みまたはキャンセル済み
  }

  size_t memory_available = buffer->memory_size - buffer->memory_head;
  if (size <= memory_available) {
    memcpy(buffer->memory_buffer + buffer->memory_head, data, size);
    buffer->memory_head += size;
    xSemaphoreGive(buffer->mutex);
    return 0;
  }

  memcpy(buffer->memory_buffer + buffer->memory_head, data, memory_available);
  buffer->memory_head += memory_available;

  size_t remaining = size - memory_available;
  if (buffer->file_size + remaining > buffer->file_max_size) {
    buffer->cancelled = true;
    xSemaphoreGive(buffer->mutex);
    return -1;  // ファイル容量オーバー
  }

  fwrite(data + memory_available, 1, remaining, buffer->file);
  buffer->file_size += remaining;

  xSemaphoreGive(buffer->mutex);
  return 0;
}

// データの読み込み関数
int ring_buffer_read(RingBuffer *buffer, uint8_t *data, size_t size) {
  xSemaphoreTake(buffer->mutex, portMAX_DELAY);
  while (!buffer->write_finished && !buffer->cancelled &&
         buffer->memory_head - buffer->memory_tail < size) {
    xSemaphoreGive(buffer->mutex);
    vTaskDelay(1 / portTICK_PERIOD_MS);  // 少し待つ
    xSemaphoreTake(buffer->mutex, portMAX_DELAY);
  }

  if (buffer->cancelled) {
    xSemaphoreGive(buffer->mutex);
    return -1;  // キャンセル済み
  }

  size_t memory_available = buffer->memory_head - buffer->memory_tail;
  size_t to_read = size < memory_available ? size : memory_available;
  memcpy(data, buffer->memory_buffer + buffer->memory_tail, to_read);
  buffer->memory_tail += to_read;

  xSemaphoreGive(buffer->mutex);
  return to_read;
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
  free(buffer->file_name);
  vSemaphoreDelete(buffer->mutex);
}
