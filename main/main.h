#ifndef MAIN_H
#define MAIN_H

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

// macros
#define MAIN_PDM_BUFFER_SIZE (I2S_BUFFER_SIZE / 4) // store buffer in long array
#define MAIN_PCM_BUFFER_SIZE (I2S_BUFFER_SIZE / 2) // store buffer in short array

// tags
#define MAIN_TAG "main"
#define MAIN_READ_TAG "read_task"
#define MAIN_WIFI_TAG "wifi_task"
#define MAIN_TIMER_TAG "timer"
#define MAIN_TCP_TAG "tcp"

// function declarations
/** @brief Read and convert I2S buffers, then queue them for the TCP sender. */
void vTaskRead(void *pvParameters);
/** @brief Send queued raw sample buffers over the configured TCP connection. */
void vTaskWifi(void *pvParameters);
/** @brief Stop the reader task when the capture timer expires. */
void vMainRecTimer(TimerHandle_t xTimer);

#endif // MAIN_H
