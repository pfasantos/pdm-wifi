/** @file main.h
 *  @brief UDP destination, capture duration and wireless task entry points.
 */
#ifndef _MAIN_H_
#define _MAIN_H_

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

/** UDP receiver IPv4 address used by main.c; Kconfig server values are unused. */
#define SERVER_IP_ADDR "10.0.0.48"
/** UDP receiver port used by main.c and udp_receiver.py. */
#define SERVER_PORT 8888

// macros
#define REC_TIME_MS 2 * 60 * 1000        // recording time
#define PDM_BUF_SIZE (BUF_SIZE / 4) // store buffer in long array
#define PCM_BUF_SIZE (BUF_SIZE / 2) // store buffer in short array

// tags
#define MAIN_TAG "main"
#define READ_TAG "read_task"
#define WIFI_TAG "wifi_task"
#define TIMER_TAG "timer"
#define TCP_TAG "tcp"
#define UDP_TAG "udp"

// function declarations
/** @brief Read and filter I2S buffers, then queue them for UDP sending.
 *  @param pvParameters Unused FreeRTOS task argument.
 */
void vTaskRead(void *pvParameters);
/** @brief Send queued raw sample buffers as UDP datagrams.
 *  @param pvParameters Unused FreeRTOS task argument.
 *  @note Recreates its socket after the reader's completion notification.
 */
void vTaskWifi(void *pvParameters);
/** @brief Notify the reader task when the capture interval expires.
 *  @param xTimerHandle Expired FreeRTOS software timer; unused by the callback.
 */
void vRecTimer(TimerHandle_t xTimerHandle);

#endif // _MAIN_H_
