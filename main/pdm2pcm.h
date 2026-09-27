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