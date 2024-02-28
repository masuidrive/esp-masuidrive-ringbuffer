#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ring_buffer.h"
#include "unity.h"

#define BUFFER_SIZE 1024
#define FILE_MAX_SIZE 2048
#define WRITE_SIZE 512
#define TASK_STACK_SIZE 2048
#define TASK_PRIORITY 5

static RingBuffer buffer;
static uint8_t memory[BUFFER_SIZE];
static const char *file_name = "test_buffer.dat";

static void write_task(void *param) {
  uint8_t data[WRITE_SIZE];
  memset(data, 0xAA, WRITE_SIZE);

  // Write more data than the buffer and file can hold to test overflow
  for (int i = 0; i < 6; ++i) {
    int result = ring_buffer_write(&buffer, data, WRITE_SIZE);
    if (i >= 4) {
      // Expect failure due to file size limit
      TEST_ASSERT_EQUAL(-1, result);
    } else {
      TEST_ASSERT_EQUAL(0, result);
    }
  }

  ring_buffer_finish_write(&buffer);
  vTaskDelete(NULL);
}

static void read_task(void *param) {
  uint8_t data[WRITE_SIZE];
  size_t total_read = 0;

  while (true) {
    int read = ring_buffer_read(&buffer, data, WRITE_SIZE);
    if (read <= 0) {
      break;
    }
    total_read += read;

    for (int i = 0; i < read; ++i) {
      TEST_ASSERT_EQUAL_HEX8(0xAA, data[i]);
    }
  }

  // Expect to read only up to the file size limit
  TEST_ASSERT_EQUAL(FILE_MAX_SIZE, total_read);
  vTaskDelete(NULL);
}

static void test_ring_buffer_overflow(void) {
  ring_buffer_init(&buffer, memory, BUFFER_SIZE, file_name, FILE_MAX_SIZE);

  xTaskCreate(write_task, "write_task", TASK_STACK_SIZE, NULL, TASK_PRIORITY,
              NULL);
  xTaskCreate(read_task, "read_task", TASK_STACK_SIZE, NULL, TASK_PRIORITY,
              NULL);

  // Wait for tasks to complete
  vTaskDelay(pdMS_TO_TICKS(3000));

  ring_buffer_free(&buffer);
}

static void test_ring_buffer_init_failure(void) {
  // Test with NULL memory buffer
  TEST_ASSERT_EQUAL(
      NULL, ring_buffer_init(NULL, BUFFER_SIZE, file_name, FILE_MAX_SIZE));

  // Test with zero buffer size
  TEST_ASSERT_EQUAL(NULL,
                    ring_buffer_init(memory, 0, file_name, FILE_MAX_SIZE));
}

static void test_ring_buffer_cancel(void) {
  ring_buffer_init(&buffer, memory, BUFFER_SIZE, file_name, FILE_MAX_SIZE);

  // Cancel the buffer before any operations
  ring_buffer_cancel(&buffer);

  // Write and read operations should fail
  uint8_t data[WRITE_SIZE];
  memset(data, 0xAA, WRITE_SIZE);
  TEST_ASSERT_EQUAL(-1, ring_buffer_write(&buffer, data, WRITE_SIZE));
  TEST_ASSERT_EQUAL(-1, ring_buffer_read(&buffer, data, WRITE_SIZE));

  ring_buffer_free(&buffer);
}

void app_main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_ring_buffer_overflow);
  RUN_TEST(test_ring_buffer_init_failure);
  RUN_TEST(test_ring_buffer_cancel);

  UNITY_END();
}
