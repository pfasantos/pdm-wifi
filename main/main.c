#include <stdio.h>
#include <errno.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "esp_log.h"
#include "driver/i2s_std.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#include "esp_netif.h"
#include "esp_event.h"

#include "lwip/iSocketError.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include "i2s_std.h"
#include "pdm2pcm.h"
#include "main.h"

#include "esp_timer.h"

// handles
static QueueHandle_t xPcmQueue;
static TimerHandle_t xRecordingTimer;
static TaskHandle_t xReaderTask;
static TaskHandle_t xWifiTask;

// filter structure
static app_cic_t xCic;

// buffers
static long plPdmBuffer[MAIN_PDM_BUFFER_SIZE];
static short psWifiBuffer[MAIN_PCM_BUFFER_SIZE];
static short psPcmBuffer[MAIN_PCM_BUFFER_SIZE];

// clock reconfig
static i2s_std_clk_config_t xRecordingClockConfig =
    I2S_STD_CLK_DEFAULT_CONFIG(CONFIG_PDM_I2S_RECORD_RATE_HZ);

// TASKS SECTION --------------------------

void vTaskRead(void *pvParameters)
{
    int iReadCount = 0;
    ESP_LOGI(MAIN_READ_TAG, "Leitura I2S iniciada");

    printf("%lld", esp_timer_get_time());
    xTimerStart(xRecordingTimer, 0);
    for (;;)
    {
        if (ulTaskNotifyTake(pdTRUE, 0) != 0)
        {
            break;
        }

        // wait untill plPdmBuffer is full
        if (i2s_channel_read(xRxHandle, (void *)plPdmBuffer, I2S_BUFFER_SIZE, NULL,
                             portMAX_DELAY) == ESP_OK)
        {
            process_app_cic(&xCic, &plPdmBuffer, &psPcmBuffer);
            process_new_fir(&psPcmBuffer);
            xQueueSend(xPcmQueue, &psPcmBuffer, portMAX_DELAY);
            iReadCount++;
        }
        else
        {
            ESP_LOGE(I2S_TAG, "Erro durante a leitura: errno %d", errno);
            break;
        }
    }
    ESP_LOGI(MAIN_READ_TAG, "Leitura I2S terminada: %d blocos lidos", iReadCount);

    printf("%lld", esp_timer_get_time());
    vI2SStdStop();

    xTaskNotifyGive(xWifiTask);
    vTaskDelete(NULL);
}

void vTaskWifi(void *pvParameters)
{
    int iSentBlocks = 0;

    struct sockaddr_in xDestinationAddress = {
        .sin_addr.s_addr = inet_addr(CONFIG_SERVER_IP_ADDR),
        .sin_family = AF_INET,
        .sin_port = htons(CONFIG_SERVER_PORT),
    };

    for (;;)
    {
        // criar socket UDP
        int iSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (iSocket < 0)
        {
            ESP_LOGE(MAIN_TCP_TAG, "Falha ao criar socket: errno %d", errno);
            break;
        }
        ESP_LOGI(MAIN_TCP_TAG, "TCP socket connected to %s:%d", CONFIG_SERVER_IP_ADDR,
                 CONFIG_SERVER_PORT);

        int iSocketError =
            connect(iSocket, (struct sockaddr *)&xDestinationAddress, sizeof(xDestinationAddress));
        if (iSocketError != 0)
        {
            ESP_LOGE(MAIN_TCP_TAG, "Falha ao conectar: errno %d", errno);
            close(iSocket);
            break;
        }
        ESP_LOGI(MAIN_TCP_TAG, "TCP connected to %s:%d", CONFIG_SERVER_IP_ADDR, CONFIG_SERVER_PORT);

        for (;;)
        {
            if ((ulTaskNotifyTake(pdTRUE, 0) != 0) && (uxQueueMessagesWaiting(xPcmQueue) != 0))
            {
                break;
            }

            if ((xPcmQueue != NULL) && (xQueueReceive(xPcmQueue, &psWifiBuffer, 0) == pdTRUE))
            {
                // send buffer
                int iSocketError = send(iSocket, psWifiBuffer, I2S_BUFFER_SIZE, 0);
                if (iSocketError < 0)
                {
                    ESP_LOGE(MAIN_TCP_TAG, "Erro durante o envio: errno %d", errno);
                    break;
                }
                iSentBlocks++;
            }
            vTaskDelay(1);
        }

        if (iSocket != -1)
        {
            ESP_LOGE(MAIN_TCP_TAG, "Closing TCP socket and restarting...");
            ESP_LOGI(MAIN_TCP_TAG, "enviou %d blocos", iSentBlocks);
            shutdown(iSocket, 0);
            close(iSocket);
        }
    }
    vTaskDelete(NULL);
}

// TIMERS SECTION --------------------------

void vMainRecTimer(TimerHandle_t xTimer)
{
    xTaskNotifyGive(xReaderTask);
    ESP_LOGI(MAIN_TIMER_TAG, "Tempo de gravacao acabou.");
}

// FUNCTIONS SECTION ------------------------

// MAIN SETUP SECTION -----------------------

void app_main(void)
{
    vI2SStdInit();

    init_app_cic(&xCic);

    // i2s init in lower clock to prevent mic damage
    i2s_channel_enable(xRxHandle);
    vTaskDelay(pdMS_TO_TICKS(5));
    i2s_channel_disable(xRxHandle);
    i2s_channel_reconfig_std_clock(xRxHandle, &xRecordingClockConfig);
    i2s_channel_enable(xRxHandle);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    xPcmQueue = xQueueCreate(CONFIG_PDM_DMA_BUFFER_COUNT, MAIN_PCM_BUFFER_SIZE * sizeof(short));
    if (xPcmQueue == NULL)
    {
        ESP_LOGE(MAIN_TAG, "Falha em criar fila de dados");
        for (;;)
            ;
    }

    xRecordingTimer =
        xTimerCreate("REC timer", pdMS_TO_TICKS(CONFIG_PDM_RECORDING_DURATION_SECONDS * 1000U),
                     pdFALSE, (void *)0, vMainRecTimer);

    if (xRecordingTimer == NULL)
    {
        ESP_LOGE(MAIN_TAG, "Falha ao criar o timer");
        for (;;)
            ;
    }

    BaseType_t xTaskCreateStatus[2];
    xTaskCreateStatus[0] =
        xTaskCreatePinnedToCore(vTaskRead, "taskREAD", configMINIMAL_STACK_SIZE + 4096, NULL,
                                configMAX_PRIORITIES - 2, &xReaderTask, APP_CPU_NUM);

    xTaskCreateStatus[1] =
        xTaskCreatePinnedToCore(vTaskWifi, "taskWifi", configMINIMAL_STACK_SIZE + 4096, NULL,
                                configMAX_PRIORITIES - 3, &xWifiTask, PRO_CPU_NUM);

    // test tasks creation
    for (int i = 0; i < 2; i++)
    {
        if (xTaskCreateStatus[i] == pdFAIL)
        {
            ESP_LOGE(MAIN_TAG, "Erro ao criar a task %d", i);
            for (;;)
                ;
        }
    }

    ESP_LOGI(MAIN_TAG, "Gravacao iniciada");
}
