#include "ring_buffer.h"
#include <memory.h>

void _ring_buffer_create_file(FILE *file, size_t file_size) {
  // ファイルサイズを確認し、必要に応じて0で埋める
  fseek(file, 0, SEEK_END);
  size_t current_size = ftell(file);
  if (current_size < file_size) {
    uint8_t zero = 0;
    for (size_t i = current_size; i < file_size; i++) {
      fwrite(&zero, 1, 1, file);
    }
    fflush(file);
  }
  fseek(file, 0, SEEK_SET); // ファイルポインタを先頭に戻す
}

// ファイルの書き込み関数
int _ring_buffer_file_write(RingBuffer *buffer, const uint8_t *data, size_t size) {
  if (buffer->write_finished) {
    return RING_BUFFER_FINISHED;
  }
  if (buffer->cancelled) {
    return RING_BUFFER_CANCELED;
  }

  size_t written = 0;
  for (size_t i = 0; i < size; ++i) {
    size_t next_head = (buffer->file_head + 1) % buffer->file_size;
    if (next_head == buffer->file_tail) {
      // ファイルがいっぱいの場合
      return written; // 書き込んだバイト数を返す
    }
    fseek(buffer->file, buffer->file_head, SEEK_SET);
    fwrite(&data[i], 1, 1, buffer->file);
    buffer->file_head = next_head;
    written++;
  }

  return RING_BUFFER_OK;
}

// ファイルの読み込み関数
int _ring_buffer_file_read(RingBuffer *buffer, uint8_t *data, size_t size) {
  if (buffer->cancelled) {
    return RING_BUFFER_CANCELED;
  }

  size_t read_count = 0;
  while (read_count < size && buffer->file_tail != buffer->file_head) {
    fseek(buffer->file, buffer->file_tail, SEEK_SET);
    fread(&data[read_count++], 1, 1, buffer->file);
    buffer->file_tail = (buffer->file_tail + 1) % buffer->file_size;
  }

  return read_count;
}
