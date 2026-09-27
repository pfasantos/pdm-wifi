/** @file i2s_std.h
 *  @brief I2S standard-mode receive channel settings and lifecycle.
 *
 *  The channel uses 32-bit stereo slots with an external data input.
 *  Initialization and shutdown functions currently do not return ESP-IDF errors.
 */
#ifndef _I2S_STD_H_
#define _I2S_STD_H_

#include "driver/i2s_std.h"

//tags
#define I2S_TAG "i2s"

//macros
#define BIT_DEPTH I2S_DATA_BIT_WIDTH_32BIT                 // i2s bit depth
#define DMA_BUF_NUM 8                                     // quantity of dma buffers 
#define DMA_BUF_SIZE 511                                   // number of samples of dma buffer 
#define BUF_SIZE (2 * DMA_BUF_SIZE * BIT_DEPTH / 8)        // size in bytes of i2s buffer 

/** Receive channel handle created by i2s_init(); valid until i2s_stop(). */
extern i2s_chan_handle_t rx_handle;

/** @brief Create and configure the I2S RX channel.
 *  @warning The implementation does not check driver return codes. Check the
 *  ESP-IDF log and channel state before relying on a successful setup.
 */
void i2s_init();
/** @brief Disable and delete the I2S RX channel.
 *  @pre The channel was created by i2s_init() and enabled for recording.
 */
void i2s_stop();

#endif // _I2S_STD_H_