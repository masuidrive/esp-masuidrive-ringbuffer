#pragma once
/*
# RingBuffer ライブラリ

LICENSE: Apache-2.0 (https://www.apache.org/licenses/LICENSE-2.0)
AUTHOR: Yuichiro Masui http://github.com/masuidrive

このライブラリは、ESP-IDF環境で動作する、汎用的なFIFOリングバッファを提供します。
メモリとディスクの2段構造を持ち、データの読み書きをサポートするとともに、
書き込み終了とキャンセルの通知機能も備えています。
また、FreeRTOSのセマフォを使用して、異なるタスクからの同時アクセスを
安全に行えるように設計されています。

主な特徴は以下の通りです：

- メモリとディスクの2段構造を持つFIFOリングバッファ。
- 書き込み終了とキャンセルの通知機能。
- FreeRTOSのセマフォを使用したスレッドセーフな設計。
- メモリバッファと`RingBuffer`構造体のポインタを引数で受け取る初期化関数。
- データの読み込みは指定したバイト数を読み込むか終了するまで待つ設計。
- ファイルサイズオーバー時のキャンセル処理。

このライブラリは、ESP-IDF環境でのIoTデバイス開発において、
データのバッファリングや非同期処理を行う際に便利です。

## 関数群のドキュメント

### `void ring_buffer_init(RingBuffer *buffer, uint8_t *memory, size_t memory_size, const char *file_name, size_t
file_size)` リングバッファを初期化します。メモリバッファとファイルバッファのサイズを指定して初期化を行います。

- buffer: RingBuffer構造体のポインタ。
- memory: メモリバッファのポインタ。
- memory_size: メモリバッファのサイズ。
- file_name: 使用するファイルの名前。
- file_size: ファイルバッファのサイズ。

### `int ring_buffer_write(RingBuffer *buffer, const uint8_t *data, size_t size)
データをリングバッファに書き込みます。メモリバッファがいっぱいの場合はファイルバッファに書き込みます。

- buffer: RingBuffer構造体のポインタ。
- data: 書き込むデータのポインタ。
- size: 書き込むデータのサイズ。

戻り値は、書き込みの結果を示す定数 (RING_BUFFER_OK, RING_BUFFER_OVERFLOW, RING_BUFFER_FINISHED, RING_BUFFER_CANCELED)
です。

### `int ring_buffer_read(RingBuffer *buffer, uint8_t *data, size_t size, TickType_t xTicksToWait)
リングバッファからデータを読み込みます。指定したバイト数を読み込むまで指定時間待ちます。

- buffer: RingBuffer構造体のポインタ。
- data: 読み込んだデータを格納するバッファのポインタ。
- size: 読み込むバイト数。
- xTicksToWait: 待機する時間（FreeRTOSのTick単位）。

戻り値は、実際に読み込んだバイト数です。キャンセルされた場合は RING_BUFFER_CANCELED を返します。

### `void ring_buffer_finish_write(RingBuffer *buffer)
リングバッファへの書き込みを終了します。書き込み終了後は、ring_buffer_write 関数は RING_BUFFER_FINISHED を返します。

buffer: RingBuffer構造体のポインタ。

### `void ring_buffer_cancel(RingBuffer *buffer)
リングバッファの操作をキャンセルします。キャンセル後は、ring_buffer_write および ring_buffer_read 関数は
RING_BUFFER_CANCELED を返します。

- buffer: RingBuffer構造体のポインタ。

### `void ring_buffer_free(RingBuffer *buffer)
リングバッファを解放します。使用していたリソースを解放します。

- buffer: RingBuffer構造体のポインタ。
*/

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

typedef struct {
  uint8_t *memory_buffer;
  size_t memory_size;
  size_t memory_head;
  size_t memory_len;

  FILE *file;
  size_t file_size;
  size_t file_head;
  size_t file_len;

  bool write_finished;
  bool cancelled;

  SemaphoreHandle_t mutex;
} RingBuffer;

#define RING_BUFFER_OK -1
#define RING_BUFFER_FINISHED -2
#define RING_BUFFER_CANCELED -3
#define RING_BUFFER_OVERFLOW 1

void ring_buffer_init(RingBuffer *buffer, uint8_t *memory, size_t memory_size, const char *file_name, size_t file_size);
int ring_buffer_write(RingBuffer *buffer, const uint8_t *data, size_t size);
int ring_buffer_read(RingBuffer *buffer, uint8_t *data, size_t size, TickType_t xTicksToWait);
void ring_buffer_finish_write(RingBuffer *buffer);
void ring_buffer_cancel(RingBuffer *buffer);
void ring_buffer_free(RingBuffer *buffer);
