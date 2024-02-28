#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "ring_buffer.h"
#include "unity.h"
#include <string.h>

// Assume these are defined somewhere
#define RING_BUFFER_SIZE 128
#define FILE_MAX_SIZE 1024
#define TEST_FILE_NAME "test_buffer_file"

TEST_CASE("ring buffer init / deinit", "[ring_buffer]") {
  uint8_t memory[RING_BUFFER_SIZE];
  RingBuffer buffer;
  ring_buffer_init(&buffer, memory, RING_BUFFER_SIZE, TEST_FILE_NAME, FILE_MAX_SIZE);
  ring_buffer_free(&buffer);
}

TEST_CASE("ring buffer write and read", "[ring_buffer]") {
  uint8_t memory[RING_BUFFER_SIZE];
  RingBuffer buffer;
  ring_buffer_init(&buffer, memory, RING_BUFFER_SIZE, TEST_FILE_NAME, FILE_MAX_SIZE);

  const char *write_data = "Hello, world!";
  size_t write_size = strlen(write_data);
  TEST_ASSERT_EQUAL(0, ring_buffer_write(&buffer, (const uint8_t *)write_data, write_size));

  uint8_t read_data[RING_BUFFER_SIZE];
  size_t read_size = ring_buffer_read(&buffer, read_data, write_size);
  TEST_ASSERT_EQUAL(write_size, read_size);
  TEST_ASSERT_EQUAL_MEMORY(write_data, read_data, write_size);

  ring_buffer_free(&buffer);
}

TEST_CASE("ring buffer write overflow", "[ring_buffer]") {
  uint8_t memory[RING_BUFFER_SIZE];
  RingBuffer buffer;
  ring_buffer_init(&buffer, memory, RING_BUFFER_SIZE, TEST_FILE_NAME, FILE_MAX_SIZE);

  uint8_t write_data[RING_BUFFER_SIZE + FILE_MAX_SIZE + 1];
  memset(write_data, 'A', sizeof(write_data));
  TEST_ASSERT_EQUAL(-1, ring_buffer_write(&buffer, write_data, sizeof(write_data)));

  ring_buffer_free(&buffer);
}

TEST_CASE("ring buffer cancel", "[ring_buffer]") {
  uint8_t memory[RING_BUFFER_SIZE];
  RingBuffer buffer;
  ring_buffer_init(&buffer, memory, RING_BUFFER_SIZE, TEST_FILE_NAME, FILE_MAX_SIZE);

  ring_buffer_cancel(&buffer);

  const char *write_data = "Hello, world!";
  size_t write_size = strlen(write_data);
  TEST_ASSERT_EQUAL(-1, ring_buffer_write(&buffer, (const uint8_t *)write_data, write_size));

  uint8_t read_data[RING_BUFFER_SIZE];
  TEST_ASSERT_EQUAL(-1, ring_buffer_read(&buffer, read_data, write_size));

  ring_buffer_free(&buffer);
}

void app_main(void) {
  UNITY_BEGIN();
  unity_run_all_tests();
  UNITY_END();
}
