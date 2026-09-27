#include "i2s_std.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2s_pdm.h"

i2s_chan_handle_t xRxHandle;

void vI2SStdInit()
{
    // i2s channel configuration
    i2s_chan_config_t xChannelConfig = {
        .id = I2S_NUM_AUTO,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = CONFIG_PDM_DMA_BUFFER_COUNT,
        .dma_frame_num = CONFIG_PDM_DMA_FRAME_COUNT,
        .auto_clear = false,
    };

    i2s_new_channel(&xChannelConfig, NULL, &xRxHandle);

    // i2s std mode configuration
    i2s_std_config_t xStandardConfig = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(
            CONFIG_PDM_I2S_START_RATE_HZ), // start channel in slower clock
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_BIT_DEPTH, I2S_SLOT_MODE_STEREO),
        .gpio_cfg =
            {
                .mclk = I2S_GPIO_UNUSED,
                .bclk = GPIO_NUM_5,
                .ws = I2S_GPIO_UNUSED,
                .dout = I2S_GPIO_UNUSED,
                .din = GPIO_NUM_4,
                .invert_flags =
                    {
                        .mclk_inv = false,
                        .bclk_inv = false,
                        .ws_inv = false,
                    },
            },
    };

    i2s_channel_init_std_mode(xRxHandle, &xStandardConfig);
    ESP_LOGI(I2S_TAG, "Canal I2S iniciado.");
}

void vI2SStdStop()
{
    i2s_channel_disable(xRxHandle);
    i2s_del_channel(xRxHandle);
    ESP_LOGI(I2S_TAG, "Canal I2S desativado.");
}
