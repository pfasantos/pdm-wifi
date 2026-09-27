/** @file i2s_std.h
 *  @brief I2S standard-mode receive channel settings and lifecycle.
 *
 *  I2S rates and DMA dimensions are set through ESP-IDF menuconfig options.
 */
#ifndef I2S_STD_H
#define I2S_STD_H

#include "sdkconfig.h"

#include "driver/i2s_std.h"

#define I2S_TAG "i2s"
#define I2S_BIT_DEPTH I2S_DATA_BIT_WIDTH_32BIT
#define I2S_BUFFER_SIZE (2U * CONFIG_PDM_DMA_FRAME_COUNT * I2S_BIT_DEPTH / 8U)

/** Receive channel handle created by vI2SStdInit(); valid until vI2SStdStop(). */
extern i2s_chan_handle_t xRxHandle;

/** @brief Create and configure the I2S RX channel. */
void vI2SStdInit(void);
/** @brief Disable and delete the I2S RX channel. */
void vI2SStdStop(void);

#endif // I2S_STD_H
