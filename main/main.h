#ifndef _MAIN_H_
#define _MAIN_H_

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#define SERVER_IP_ADDR "10.0.0.48" 
#define SERVER_PORT 8888

//macros
#define REC_TIME_MS     5 * 1000         // recording time
#define PDM_BUF_SIZE    (BUF_SIZE/4)      // store buffer in long array
#define PCM_BUF_SIZE    (BUF_SIZE/2)      // store buffer in short array

// tags
#define MAIN_TAG  "main"
#define READ_TAG  "read_task"
#define WIFI_TAG  "wifi_task"
#define TIMER_TAG "timer"
#define TCP_TAG   "tcp"
#define UDP_TAG   "udp"

// function declarations
void vTaskRead(void *pvParameters);
void vTaskWifi(void *pvParameters);
void vRecTimer(TimerHandle_t xTimerHandle);

#endif // _MAIN_H_