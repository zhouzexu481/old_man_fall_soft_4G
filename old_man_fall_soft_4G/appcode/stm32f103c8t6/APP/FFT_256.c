#include "FFT_256.h"
#include "string.h"

#define BLOCK_SIZE           1     /* 调用一次arm_fir_f32处理的采样点个数 */
#define NUM_TAPS             29     /* 滤波器系数个数 */

uint32_t blockSize = BLOCK_SIZE;
uint32_t numBlocks = BLOCK_SIZE;            /* 需要调用arm_fir_f32的次数 */
arm_fir_instance_f32 S_ir,S_red;
static float firStateF32_ir[BLOCK_SIZE + NUM_TAPS - 1];        /* 状态缓存，大小numTaps + blockSize - 1*/
static float firStateF32_red[BLOCK_SIZE + NUM_TAPS - 1];        /* 状态缓存，大小numTaps + blockSize - 1*/


/* 低通滤波器系数 通过fadtool获取*/
const float firCoeffs32LP[NUM_TAPS] = {
  -0.001542701735,-0.002211477375,-0.003286228748, -0.00442651147,-0.004758632276,
  -0.003007677384, 0.002192312852,  0.01188309677,  0.02637642808,  0.04498152807,
    0.06596207619,   0.0867607221,   0.1044560149,   0.1163498312,   0.1205424443,
     0.1163498312,   0.1044560149,   0.0867607221,  0.06596207619,  0.04498152807,
    0.02637642808,  0.01188309677, 0.002192312852,-0.003007677384,-0.004758632276,
   -0.00442651147,-0.003286228748,-0.002211477375,-0.001542701735
};

void arm_fir_init_f32(
  arm_fir_instance_f32 * S,
  uint16_t numTaps,
  float * pCoeffs,
  float * pState,
  uint32_t blockSize)
{
  /* Assign filter taps */
  S->numTaps = numTaps;

  /* Assign coefficient pointer */
  S->pCoeffs = pCoeffs;

  /* Clear state buffer and the size of state buffer is (blockSize + numTaps - 1) */
  memset(pState, 0, (numTaps + (blockSize - 1u)) * sizeof(float));

  /* Assign state pointer */
  S->pState = pState;

}


void arm_fir_f32(
const arm_fir_instance_f32 * S,
float * pSrc,
float * pDst,
uint32_t blockSize)
{
   float *pState = S->pState;                 /* State pointer */
   float *pCoeffs = S->pCoeffs;               /* Coefficient pointer */
   float *pStateCurnt;                        /* Points to the current sample of the state */
   float *px, *pb;                            /* Temporary pointers for state and coefficient buffers */
   float acc0, acc1, acc2, acc3, acc4, acc5, acc6, acc7;     /* Accumulators */
   float x0, x1, x2, x3, x4, x5, x6, x7, c0;  /* Temporary variables to hold state and coefficient values */
   uint32_t numTaps = S->numTaps;                 /* Number of filter coefficients in the filter */
   uint32_t i, tapCnt, blkCnt;                    /* Loop counters */
   float p0,p1,p2,p3,p4,p5,p6,p7;             /* Temporary product values */

   /* S->pState points to state array which contains previous frame (numTaps - 1) samples */
   /* pStateCurnt points to the location where the new input data should be written */
   pStateCurnt = &(S->pState[(numTaps - 1u)]);

   /* Apply loop unrolling and compute 8 output values simultaneously.  
    * The variables acc0 ... acc7 hold output values that are being computed:  
    *  
    *    acc0 =  b[numTaps-1] * x[n-numTaps-1] + b[numTaps-2] * x[n-numTaps-2] + b[numTaps-3] * x[n-numTaps-3] +...+ b[0] * x[0]  
    *    acc1 =  b[numTaps-1] * x[n-numTaps] +   b[numTaps-2] * x[n-numTaps-1] + b[numTaps-3] * x[n-numTaps-2] +...+ b[0] * x[1]  
    *    acc2 =  b[numTaps-1] * x[n-numTaps+1] + b[numTaps-2] * x[n-numTaps] +   b[numTaps-3] * x[n-numTaps-1] +...+ b[0] * x[2]  
    *    acc3 =  b[numTaps-1] * x[n-numTaps+2] + b[numTaps-2] * x[n-numTaps+1] + b[numTaps-3] * x[n-numTaps]   +...+ b[0] * x[3]  
    */
   blkCnt = blockSize >> 3;

   /* First part of the processing with loop unrolling.  Compute 8 outputs at a time.  
   ** a second loop below computes the remaining 1 to 7 samples. */
   while(blkCnt > 0u)
   {
      /* Copy four new input samples into the state buffer */
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;

      /* Set all accumulators to zero */
      acc0 = 0.0f;
      acc1 = 0.0f;
      acc2 = 0.0f;
      acc3 = 0.0f;
      acc4 = 0.0f;
      acc5 = 0.0f;
      acc6 = 0.0f;
      acc7 = 0.0f;		

      /* Initialize state pointer */
      px = pState;

      /* Initialize coeff pointer */
      pb = (pCoeffs);		
   
      /* This is separated from the others to avoid 
       * a call to __aeabi_memmove which would be slower
       */
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;
      *pStateCurnt++ = *pSrc++;

      /* Read the first seven samples from the state buffer:  x[n-numTaps], x[n-numTaps-1], x[n-numTaps-2] */
      x0 = *px++;
      x1 = *px++;
      x2 = *px++;
      x3 = *px++;
      x4 = *px++;
      x5 = *px++;
      x6 = *px++;

      /* Loop unrolling.  Process 8 taps at a time. */
      tapCnt = numTaps >> 3u;
      
      /* Loop over the number of taps.  Unroll by a factor of 8.  
       ** Repeat until we've computed numTaps-8 coefficients. */
      while(tapCnt > 0u)
      {
         /* Read the b[numTaps-1] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-3] sample */
         x7 = *(px++);

         /* acc0 +=  b[numTaps-1] * x[n-numTaps] */
         p0 = x0 * c0;

         /* acc1 +=  b[numTaps-1] * x[n-numTaps-1] */
         p1 = x1 * c0;

         /* acc2 +=  b[numTaps-1] * x[n-numTaps-2] */
         p2 = x2 * c0;

         /* acc3 +=  b[numTaps-1] * x[n-numTaps-3] */
         p3 = x3 * c0;

         /* acc4 +=  b[numTaps-1] * x[n-numTaps-4] */
         p4 = x4 * c0;

         /* acc1 +=  b[numTaps-1] * x[n-numTaps-5] */
         p5 = x5 * c0;

         /* acc2 +=  b[numTaps-1] * x[n-numTaps-6] */
         p6 = x6 * c0;

         /* acc3 +=  b[numTaps-1] * x[n-numTaps-7] */
         p7 = x7 * c0;
         
         /* Read the b[numTaps-2] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-4] sample */
         x0 = *(px++);
         
         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;


         /* Perform the multiply-accumulate */
         p0 = x1 * c0;
         p1 = x2 * c0;   
         p2 = x3 * c0;   
         p3 = x4 * c0;   
         p4 = x5 * c0;   
         p5 = x6 * c0;   
         p6 = x7 * c0;   
         p7 = x0 * c0;   
         
         /* Read the b[numTaps-3] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-5] sample */
         x1 = *(px++);
         
         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Perform the multiply-accumulates */      
         p0 = x2 * c0;
         p1 = x3 * c0;   
         p2 = x4 * c0;   
         p3 = x5 * c0;   
         p4 = x6 * c0;   
         p5 = x7 * c0;   
         p6 = x0 * c0;   
         p7 = x1 * c0;   

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x2 = *(px++);
         
         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Perform the multiply-accumulates */      
         p0 = x3 * c0;
         p1 = x4 * c0;   
         p2 = x5 * c0;   
         p3 = x6 * c0;   
         p4 = x7 * c0;   
         p5 = x0 * c0;   
         p6 = x1 * c0;   
         p7 = x2 * c0;   

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x3 = *(px++);
         
         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Perform the multiply-accumulates */      
         p0 = x4 * c0;
         p1 = x5 * c0;   
         p2 = x6 * c0;   
         p3 = x7 * c0;   
         p4 = x0 * c0;   
         p5 = x1 * c0;   
         p6 = x2 * c0;   
         p7 = x3 * c0;   

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x4 = *(px++);
         
         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Perform the multiply-accumulates */      
         p0 = x5 * c0;
         p1 = x6 * c0;   
         p2 = x7 * c0;   
         p3 = x0 * c0;   
         p4 = x1 * c0;   
         p5 = x2 * c0;   
         p6 = x3 * c0;   
         p7 = x4 * c0;   

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x5 = *(px++);
         
         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Perform the multiply-accumulates */      
         p0 = x6 * c0;
         p1 = x7 * c0;   
         p2 = x0 * c0;   
         p3 = x1 * c0;   
         p4 = x2 * c0;   
         p5 = x3 * c0;   
         p6 = x4 * c0;   
         p7 = x5 * c0;   

         /* Read the b[numTaps-4] coefficient */
         c0 = *(pb++);

         /* Read x[n-numTaps-6] sample */
         x6 = *(px++);
         
         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Perform the multiply-accumulates */      
         p0 = x7 * c0;
         p1 = x0 * c0;   
         p2 = x1 * c0;   
         p3 = x2 * c0;   
         p4 = x3 * c0;   
         p5 = x4 * c0;   
         p6 = x5 * c0;   
         p7 = x6 * c0;   

         tapCnt--;
         
         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;
      }

      /* If the filter length is not a multiple of 8, compute the remaining filter taps */
      tapCnt = numTaps % 0x8u;

      while(tapCnt > 0u)
      {
         /* Read coefficients */
         c0 = *(pb++);

         /* Fetch 1 state variable */
         x7 = *(px++);

         /* Perform the multiply-accumulates */      
         p0 = x0 * c0;
         p1 = x1 * c0;   
         p2 = x2 * c0;   
         p3 = x3 * c0;   
         p4 = x4 * c0;   
         p5 = x5 * c0;   
         p6 = x6 * c0;   
         p7 = x7 * c0;   

         /* Reuse the present sample states for next sample */
         x0 = x1;
         x1 = x2;
         x2 = x3;
         x3 = x4;
         x4 = x5;
         x5 = x6;
         x6 = x7;
         
         acc0 += p0;
         acc1 += p1;
         acc2 += p2;
         acc3 += p3;
         acc4 += p4;
         acc5 += p5;
         acc6 += p6;
         acc7 += p7;

         /* Decrement the loop counter */
         tapCnt--;
      }

      /* Advance the state pointer by 8 to process the next group of 8 samples */
      pState = pState + 8;

      /* The results in the 8 accumulators, store in the destination buffer. */
      *pDst++ = acc0;
      *pDst++ = acc1;
      *pDst++ = acc2;
      *pDst++ = acc3;
      *pDst++ = acc4;
      *pDst++ = acc5;
      *pDst++ = acc6;
      *pDst++ = acc7;

      blkCnt--;
   }

   /* If the blockSize is not a multiple of 8, compute any remaining output samples here.  
   ** No loop unrolling is used. */
   blkCnt = blockSize % 0x8u;

   while(blkCnt > 0u)
   {
      /* Copy one sample at a time into state buffer */
      *pStateCurnt++ = *pSrc++;

      /* Set the accumulator to zero */
      acc0 = 0.0f;

      /* Initialize state pointer */
      px = pState;

      /* Initialize Coefficient pointer */
      pb = (pCoeffs);

      i = numTaps;

      /* Perform the multiply-accumulates */
      do
      {
         acc0 += *px++ * *pb++;
         i--;

      } while(i > 0u);

      /* The result is store in the destination buffer. */
      *pDst++ = acc0;

      /* Advance state pointer by 1 for the next sample */
      pState = pState + 1;

      blkCnt--;
   }

   /* Processing is complete.  
   ** Now copy the last numTaps - 1 samples to the start of the state buffer.  
   ** This prepares the state buffer for the next function call. */

   /* Points to the start of the state buffer */
   pStateCurnt = S->pState;

   tapCnt = (numTaps - 1u) >> 2u;

   /* copy data */
   while(tapCnt > 0u)
   {
      *pStateCurnt++ = *pState++;
      *pStateCurnt++ = *pState++;
      *pStateCurnt++ = *pState++;
      *pStateCurnt++ = *pState++;

      /* Decrement the loop counter */
      tapCnt--;
   }

   /* Calculate remaining number of copies */
   tapCnt = (numTaps - 1u) % 0x4u;

   /* Copy the remaining q31_t data */
   while(tapCnt > 0u)
   {
      *pStateCurnt++ = *pState++;

      /* Decrement the loop counter */
      tapCnt--;
   }
}


void max30102_fir_init(void)
{
	arm_fir_init_f32(&S_ir, NUM_TAPS,(float *)&firCoeffs32LP[0], &firStateF32_ir[0], blockSize);
	arm_fir_init_f32(&S_red, NUM_TAPS,(float *)&firCoeffs32LP[0], &firStateF32_red[0], blockSize);
}

void ir_max30102_fir(float *input,float *output)
{
	arm_fir_f32(&S_ir,input,output,  blockSize);
}

void red_max30102_fir(float *input,float *output)
{
	arm_fir_f32(&S_red,input,output,  blockSize);
}


