/** @file pdm2pcm.h
 *  @brief Application CIC conversion and post-filter interface.
 *
 *  This module converts packed 32-bit microphone words into signed short
 *  samples. Filter state persists across consecutive buffers.
 */
#ifndef _PDM2PCM_H_
#define _PDM2PCM_H_

#include <stdint.h>

#define SAMPLES 1022
#define APPLY_MASK(x,i) (int32_t)((((x>>(31-i))&0x00000001) << 1)-1)
#define INPUT_SAMPLE_SIZE 32

#define MAX_OVERFLOW  1073741823 // (int32_t)(pow(2,30)-1)
#define MIN_OVERFLOW -1073741824 // (int32_t)(-pow(2,30))

/** State for the two integrator and two comb stages used by the recorder. */
typedef struct
{
    int32_t acc_s1;
    int32_t acc_s2;
    int32_t prev_s1;
    int32_t prev_s2;
} app_cic_t;

/** @brief Reset the application CIC filter state.
 *  @param cic Filter state to initialize.
 */
void init_app_cic(app_cic_t *cic);

/** @brief Apply the implementation's accumulator wrap correction.
 *  @param cic Filter state whose accumulators are adjusted in place.
 */
void integ_overflow(app_cic_t *cic);

/** @brief Convert SAMPLES 32-bit PDM words into 2*SAMPLES samples.
 *  @param cic Persistent CIC state; do not reset between adjacent buffers.
 *  @param input_buffer Input array of SAMPLES packed 32-bit words.
 *  @param output_buffer Output array receiving 2*SAMPLES signed short samples.
 *  @note The function has no status return and assumes both buffers are valid.
 */
void process_app_cic(app_cic_t *cic, int32_t (*input_buffer)[SAMPLES], short (*output_buffer)[2*SAMPLES]);

/** @brief Apply the three-term post-filter to one PCM sample buffer.
 *  @param pcm_samples Buffer of 2*SAMPLES samples modified in place.
 *  @warning State is held in static variables, so calls must be serialized and
 *  consecutive recordings inherit state unless the process restarts.
 */
void process_new_fir(short (*pcm_samples)[2*SAMPLES]);

#endif // _PDM2PCM_H_
