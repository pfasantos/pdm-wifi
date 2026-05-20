/**
 * @file pdm2pcm.h
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date July 1 2022
 */

#ifndef _PDM2PCM_H_
#define _PDM2PCM_H_

#include <stdint.h>

#define SAMPLES 1022 
#define APPLY_MASK(x,i) (int32_t)((((x>>(31-i))&0x00000001) << 1)-1)
#define INPUT_SAMPLE_SIZE 32

#define MAX_OVERFLOW  1073741823 // (int32_t)(pow(2,30)-1)
#define MIN_OVERFLOW -1073741824 // (int32_t)(-pow(2,30))

typedef struct 
{
    int32_t acc_s1;
    int32_t acc_s2;
    int32_t prev_s1;
    int32_t prev_s2;
} app_cic_t;

/**
 * @brief Initialize App Specific CIC structure with default parameters.
 */
void init_app_cic(app_cic_t *cic);

/**
 * @brief Check for overflow in integrators' accumulators for App Specific CIC structure.
 */
void integ_overflow(app_cic_t *cic);

/**
 * @brief Process input buffer with App Specific CIC structure.
 */
void process_app_cic(app_cic_t *cic, int32_t (*input_buffer)[SAMPLES], short (*output_buffer)[2*SAMPLES]);

/**
 * @brief Process input buffer with a simple FIR-like differential filter.
 */
void process_new_fir(short (*pcm_samples)[2*SAMPLES]);

#endif // _PDM2PCM_H_