#pragma once

#include "ring_buffer.h"

int _ring_buffer_mem_write(RingBuffer *buffer, const uint8_t *data, size_t size);
int _ring_buffer_mem_read(RingBuffer *buffer, uint8_t *data, size_t size);
size_t _ring_buffer_mem_usage(RingBuffer *buffer);
int _ring_buffer_file_write(RingBuffer *buffer, const uint8_t *data, size_t size);
int _ring_buffer_file_read(RingBuffer *buffer, uint8_t *data, size_t size);
size_t _ring_buffer_file_usage(RingBuffer *buffer);
void _ring_buffer_create_file(FILE *file, size_t file_size);