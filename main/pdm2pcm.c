/**
 * @file pdm2pcm.c
 * @brief 
 *
 * @author Levy Gabriel da S. G.
 * @date July 1 2022
 */

#include "pdm2pcm.h"

void init_app_cic(app_cic_t *cic)
{
    cic->acc_s1 = 0;
    cic->acc_s2 = 0;
    cic->prev_s1 = 0;
    cic->prev_s2 = 0;
}

void integ_overflow(app_cic_t *cic)
{
    cic->acc_s1 = cic->acc_s1>=MAX_OVERFLOW ? cic->acc_s1-MAX_OVERFLOW : cic->acc_s1;
    cic->acc_s1 = cic->acc_s1<=MIN_OVERFLOW ? cic->acc_s1-MIN_OVERFLOW : cic->acc_s1;
    cic->acc_s2 = cic->acc_s1>=MAX_OVERFLOW ? cic->acc_s2-MAX_OVERFLOW : cic->acc_s2;
    cic->acc_s2 = cic->acc_s2<=MIN_OVERFLOW ? cic->acc_s2-MIN_OVERFLOW : cic->acc_s2;
}

void process_app_cic(app_cic_t *cic, int32_t (*input_buffer)[SAMPLES], short (*output_buffer)[2*SAMPLES])
{
    int32_t sample, temp, pcm_sample, comb_in[2];
    
    for(int ii=0; ii<SAMPLES; ii++)
    {
        sample = (*input_buffer)[ii];
        
        // 2 stages integration

        // decimation for first comb sample (avoid if statement)
        cic->acc_s1 += APPLY_MASK(sample,INPUT_SAMPLE_SIZE*ii%INPUT_SAMPLE_SIZE);
        cic->acc_s2 += cic->acc_s1;
        integ_overflow(cic);
        comb_in[0] = cic->acc_s2;

        for(int jj=INPUT_SAMPLE_SIZE*ii+1; jj<INPUT_SAMPLE_SIZE/2*(2*ii+1); jj++) 
        {
            cic->acc_s1 += APPLY_MASK(sample,jj%INPUT_SAMPLE_SIZE);
            cic->acc_s2 += cic->acc_s1;
            integ_overflow(cic);
        }

        // decimation for second comb sample (avoid if statement)
        cic->acc_s1 += APPLY_MASK(sample,(INPUT_SAMPLE_SIZE/2*(2*ii+1))%INPUT_SAMPLE_SIZE);
        cic->acc_s2 += cic->acc_s1;
        integ_overflow(cic);
        comb_in[1] = cic->acc_s2;

        for(int jj=INPUT_SAMPLE_SIZE/2*(2*ii+1)+1; jj<INPUT_SAMPLE_SIZE*(ii+1); jj++) 
        {
            cic->acc_s1 += APPLY_MASK(sample,jj%INPUT_SAMPLE_SIZE);
            cic->acc_s2 += cic->acc_s1;
            integ_overflow(cic);
        }

        for(int jj=0; jj<2; jj++)
        {
            temp = comb_in[jj] - cic->prev_s1;
            cic->prev_s1 = comb_in[jj];

            pcm_sample = temp - cic->prev_s2;
            cic->prev_s2 = temp;

            pcm_sample = pcm_sample>=MAX_OVERFLOW ? pcm_sample-MAX_OVERFLOW : pcm_sample;
            pcm_sample = pcm_sample<=MIN_OVERFLOW ? pcm_sample-MIN_OVERFLOW : pcm_sample;

            (*output_buffer)[2*ii+jj] = (short) pcm_sample;
        }
    }
}

void process_new_fir(short(*pcm_samples)[2 * SAMPLES])
{
    static int32_t x_p = 0;
    static int32_t x_pp = 0;
    int size_pcm = 2*SAMPLES;
    int32_t mult, acc, kernel;

    for (int ii=0;ii<size_pcm;ii++)
    {
        kernel = (*pcm_samples)[ii];
        (*pcm_samples)[ii] = 0;
        mult = (x_p << 3) + (x_p << 1);
        acc = kernel - mult + x_pp;
        (*pcm_samples)[ii] = (short)(acc/-8);
        
        x_pp = x_p;
        x_p = kernel; 
    }
}