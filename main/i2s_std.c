#include "i2s_std.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2s_pdm.h"

i2s_chan_handle_t rx_handle;

void i2s_init()
{
    // i2s channel configuration 
    i2s_chan_config_t chan_cfg = {
        .id = I2S_NUM_AUTO,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = DMA_BUF_NUM,
        .dma_frame_num = DMA_BUF_SIZE,
        .auto_clear = false,
    };

    i2s_new_channel(&chan_cfg, NULL, &rx_handle);

    // i2s std mode configuration
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(8000),    // start channel in slower clock
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(BIT_DEPTH, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = GPIO_NUM_21,
            .ws = I2S_GPIO_UNUSED,
            .dout = I2S_GPIO_UNUSED,
            .din = GPIO_NUM_4,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };

    i2s_channel_init_std_mode(rx_handle, &std_cfg);
    ESP_LOGI(I2S_TAG, "Canal I2S iniciado.");
}

void i2s_stop()
{
    i2s_channel_disable(rx_handle);
    i2s_del_channel(rx_handle);
    ESP_LOGI(I2S_TAG, "Canal I2S desativado.");
}