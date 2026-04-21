#ifndef  __FFT_256_H__
#define  __FFT_256_H__

#include "sys.h"



typedef struct
{
uint16_t numTaps;     /**< number of filter coefficients in the filter. */
float *pState;    /**< points to the state variable array. The array is of length numTaps+blockSize-1. */
float *pCoeffs;   /**< points to the coefficient array. The array is of length numTaps. */
} arm_fir_instance_f32;


void max30102_fir_init(void);


#endif
