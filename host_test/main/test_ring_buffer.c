#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "ring_buffer.h"
#include "unity.h"
#include <string.h>

// Assume these are defined somewhere
#define MEM_BUFFER_SIZE 128
#define FILE_MAX_SIZE 1024
#define TEST_FILE_NAME "test_buffer.dat"

TEST_CASE("ring buffer init / deinit", "[ring_buffer]") {
  uint8_t memory[MEM_BUFFER_SIZE];
  RingBuffer buffer;
  ring_buffer_init(&buffer, memory, MEM_BUFFER_SIZE, TEST_FILE_NAME, FILE_MAX_SIZE);
  ring_buffer_free(&buffer);
}

TEST_CASE("ring buffer write and read", "[ring_buffer]") {
  uint8_t memory[MEM_BUFFER_SIZE];
  RingBuffer buffer;
  ring_buffer_init(&buffer, memory, MEM_BUFFER_SIZE, TEST_FILE_NAME, FILE_MAX_SIZE);

  const char *write_data = "Hello, world!";
  size_t write_size = strlen(write_data);
  TEST_ASSERT_EQUAL(0, ring_buffer_write(&buffer, (const uint8_t *)write_data, write_size));

  uint8_t read_data[MEM_BUFFER_SIZE];
  size_t read_size = ring_buffer_read(&buffer, read_data, write_size, portMAX_DELAY);
  TEST_ASSERT_EQUAL(write_size, read_size);
  TEST_ASSERT_EQUAL_MEMORY(write_data, read_data, write_size);

  ring_buffer_free(&buffer);
}

TEST_CASE("ring buffer write and read byte by byte", "[ring_buffer]") {
  uint8_t memory[MEM_BUFFER_SIZE];
  RingBuffer buffer;
  ring_buffer_init(&buffer, memory, MEM_BUFFER_SIZE, TEST_FILE_NAME, FILE_MAX_SIZE);

  // Test with memory only
  for (int i = 0; i < MEM_BUFFER_SIZE; i++) {
    uint8_t write_data = (uint8_t)i;
    TEST_ASSERT_EQUAL(0, ring_buffer_write(&buffer, &write_data, 1));

    uint8_t read_data;
    size_t read_size = ring_buffer_read(&buffer, &read_data, 1, portMAX_DELAY);
    TEST_ASSERT_EQUAL(1, read_size);
    TEST_ASSERT_EQUAL(write_data, read_data);
  }

  // Test with file storage
  for (int i = 0; i < FILE_MAX_SIZE; i++) {
    uint8_t write_data = (uint8_t)i;
    TEST_ASSERT_EQUAL(0, ring_buffer_write(&buffer, &write_data, 1));

    uint8_t read_data;
    size_t read_size = ring_buffer_read(&buffer, &read_data, 1, portMAX_DELAY);
    TEST_ASSERT_EQUAL(1, read_size);
    TEST_ASSERT_EQUAL(write_data, read_data);
  }

  ring_buffer_free(&buffer);
}

TEST_CASE("ring buffer append and read multiple times", "[ring_buffer]") {
  uint8_t memory[MEM_BUFFER_SIZE];
  RingBuffer buffer;
  ring_buffer_init(&buffer, memory, MEM_BUFFER_SIZE, TEST_FILE_NAME, FILE_MAX_SIZE);

  const char *write_data1 = "Hello";
  const char *write_data2 = "World";
  const char *expected_result = "HelloWorld";

  // Append and read multiple times
  for (int i = 0; i < 3; i++) {
    TEST_ASSERT_EQUAL(0, ring_buffer_write(&buffer, (const uint8_t *)write_data1, strlen(write_data1)));
    TEST_ASSERT_EQUAL(0, ring_buffer_write(&buffer, (const uint8_t *)write_data2, strlen(write_data2)));

    uint8_t read_data[20];
    size_t read_size = ring_buffer_read(&buffer, read_data, strlen(expected_result), portMAX_DELAY);
    TEST_ASSERT_EQUAL(strlen(expected_result), read_size);
    TEST_ASSERT_EQUAL_MEMORY(expected_result, read_data, read_size);
  }

  ring_buffer_free(&buffer);
}

TEST_CASE("ring buffer partial read and append multiple times", "[ring_buffer]") {
  uint8_t memory[MEM_BUFFER_SIZE];
  RingBuffer buffer;
  ring_buffer_init(&buffer, memory, MEM_BUFFER_SIZE, TEST_FILE_NAME, FILE_MAX_SIZE);

  const char *write_data1 = "Hello";
  const char *write_data2 = "World";
  const char *expected_result1 = "He";
  const char *expected_result2 = "lloWorld";

  // Partial read and append multiple times
  for (int i = 0; i < 3; i++) {
    TEST_ASSERT_EQUAL(0, ring_buffer_write(&buffer, (const uint8_t *)write_data1, strlen(write_data1)));
    TEST_ASSERT_EQUAL(0, ring_buffer_write(&buffer, (const uint8_t *)write_data2, strlen(write_data2)));

    uint8_t read_data[20];
    size_t read_size = ring_buffer_read(&buffer, read_data, strlen(expected_result1), portMAX_DELAY);
    TEST_ASSERT_EQUAL(strlen(expected_result1), read_size);
    TEST_ASSERT_EQUAL_MEMORY(expected_result1, read_data, read_size);

    read_size = ring_buffer_read(&buffer, read_data, strlen(expected_result2), portMAX_DELAY);
    TEST_ASSERT_EQUAL(strlen(expected_result2), read_size);
    TEST_ASSERT_EQUAL_MEMORY(expected_result2, read_data, read_size);
  }

  ring_buffer_free(&buffer);
}

TEST_CASE("ring buffer write overflow", "[ring_buffer]") {
  uint8_t memory[MEM_BUFFER_SIZE];
  RingBuffer buffer;
  ring_buffer_init(&buffer, memory, MEM_BUFFER_SIZE, TEST_FILE_NAME, FILE_MAX_SIZE);

  uint8_t write_data[MEM_BUFFER_SIZE + FILE_MAX_SIZE + 1];
  memset(write_data, 'A', sizeof(write_data));
  TEST_ASSERT_EQUAL(RING_BUFFER_OVERFLOW, ring_buffer_write(&buffer, write_data, sizeof(write_data)));

  ring_buffer_free(&buffer);
}

TEST_CASE("ring buffer cancel", "[ring_buffer]") {
  uint8_t memory[MEM_BUFFER_SIZE];
  RingBuffer buffer;
  ring_buffer_init(&buffer, memory, MEM_BUFFER_SIZE, TEST_FILE_NAME, FILE_MAX_SIZE);

  ring_buffer_cancel(&buffer);

  const char *write_data = "Hello, world!";
  size_t write_size = strlen(write_data);
  TEST_ASSERT_EQUAL(RING_BUFFER_CANCELED, ring_buffer_write(&buffer, (const uint8_t *)write_data, write_size));

  uint8_t read_data[MEM_BUFFER_SIZE];
  TEST_ASSERT_EQUAL(RING_BUFFER_CANCELED, ring_buffer_read(&buffer, read_data, write_size, portMAX_DELAY));

  ring_buffer_free(&buffer);
}

void app_main(void) {
  UNITY_BEGIN();
  unity_run_all_tests();
  UNITY_END();
  exit(0);
}
