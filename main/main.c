#include <stdio.h>
#include <errno.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "esp_log.h"
#include "driver/i2s_std.h"

#include "i2s_std.h"
#include "pdm2pcm.h"
#include "main.h"
#include "wifi.h"

// handles
static QueueHandle_t xQueueHandle;
static TimerHandle_t xRecTimerHandle;
static TaskHandle_t xTaskReadHandle;
static TaskHandle_t xTaskWifiHandle;

//filtering structures
static app_cic_t cic;

// buffers
static long rx_buffer[PDM_BUF_SIZE];
static short sd_buffer[PCM_BUF_SIZE];
static short data_buffer[PCM_BUF_SIZE];

// clock reconfig
static i2s_std_clk_config_t clk_rec_cfg = I2S_STD_CLK_DEFAULT_CONFIG(75000);

// TASKS SECTION --------------------------

void vTaskRead(void *pvParameters)
{
    while (1)
    {
        if (ulTaskNotifyTake(pdTRUE, 0) != 0)
        {
            break;
        }

        // wait untill rx_buffer is full 
        if (i2s_channel_read(rx_handle, (void *)rx_buffer, BUF_SIZE, NULL, portMAX_DELAY) == ESP_OK)
        {
            process_app_cic(&cic, &rx_buffer, &data_buffer);
            xQueueSend(xQueueHandle, &data_buffer, portMAX_DELAY);
        }
        else
        {
            ESP_LOGE(I2S_TAG, "Erro durante a leitura: errno %d", errno);
            break;
        }
    }
    ESP_LOGI(READ_TAG, "Leitura I2S terminada");
    i2s_stop();

    xTaskNotifyGive(xTaskWifiHandle);
    vTaskDelete(NULL);
}

void vTaskWifi(void *pvParameters)
{
    while (1)
    {
        if ((xQueueHandle != NULL) && (xQueueReceive(xQueueHandle, sd_buffer, pdMS_TO_TICKS(500)) == pdTRUE))
        {
            process_new_fir(&sd_buffer);
        }
        // iriie what is left when reading ends 
        if (ulTaskNotifyTake(pdTRUE, 0) != 0)
        {
            while (uxQueueMessagesWaiting(xQueueHandle) > 0)
            {
                if (xQueueReceive(xQueueHandle, sd_buffer, 0) == pdTRUE)
                {
                    process_new_fir(&sd_buffer);
                }
            }
            break;
        }
    }

    ESP_LOGI(WIFI_TAG, "Envio finalizado");
    vTaskDelete(NULL);
}

// TIMERS SECTION --------------------------

void vRecTimer(TimerHandle_t xTimerHandle)
{
    xTaskNotifyGive(xTaskReadHandle);
    ESP_LOGI(TIMER_TAG, "Tempo de gravacao acabou.");
}

// FUNCTIONS SECTION ------------------------

// MAIN SETUP SECTION -----------------------

void app_main(void)
{
    i2s_init();
    
    i2s_channel_enable(rx_handle);
    vTaskDelay(pdMS_TO_TICKS(5));
    i2s_channel_disable(rx_handle);
    i2s_channel_reconfig_std_clock(rx_handle, &clk_rec_cfg);
    i2s_channel_enable(rx_handle);

    init_app_cic(&cic);

    xQueueHandle = xQueueCreate(DMA_BUF_NUM, PCM_BUF_SIZE * sizeof(short));
    if (xQueueHandle == NULL)
    {
        ESP_LOGE(MAIN_TAG, "Falha em criar fila de dados");
        while (1);
    }

    xRecTimerHandle = xTimerCreate(
        "REC timer",
        pdMS_TO_TICKS(REC_TIME_MS),
        pdFALSE,
        (void *)0,
        vRecTimer);

    if (xRecTimerHandle == NULL)
    {
        ESP_LOGE(MAIN_TAG, "Falha ao criar o timer");
        while (1);
    }

    BaseType_t xReturnedTask[2];
    xReturnedTask[0] = xTaskCreatePinnedToCore(
        vTaskRead,
        "taskREAD",
        configMINIMAL_STACK_SIZE + 4096,
        NULL,
        configMAX_PRIORITIES - 3,
        &xTaskReadHandle,
        APP_CPU_NUM);

    xReturnedTask[1] = xTaskCreatePinnedToCore(
        vTaskWifi,
        "taskWifi",
        configMINIMAL_STACK_SIZE + 4096,
        NULL,
        configMAX_PRIORITIES - 3,
        &xTaskWifiHandle,
        PRO_CPU_NUM);
    
    // test tasks creation
    for (int i = 0; i < 2; i++)
    {
        if (xReturnedTask[i] == pdFAIL)
        {
            ESP_LOGE(MAIN_TAG, "Erro ao criar a task %d", i);
            while (1);
        }
    }

    xTimerStart(xRecTimerHandle, 0);
    ESP_LOGI(MAIN_TAG, "Gravacao iniciada");
}