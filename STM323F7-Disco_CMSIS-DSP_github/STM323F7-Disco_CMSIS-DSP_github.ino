/*
 * Il programma compila correttamente ed i risultati sono coerenti con l'aspettative.
*/

#include "CMSIS_DSP.h"

#include "arm_math.h"
#include "arm_const_structs.h"

#include "wave_for_pico.h"
#include "hanning_int16.h"

#define FFT_SIZE  (1024)
#define BUFFER_SIZE (1024)

arm_rfft_instance_q15 varInstRfftQ15;
arm_status status;

int16_t samples_q15[BUFFER_SIZE];
int16_t cmplx_q15[BUFFER_SIZE*2];
int16_t bins_q15[BUFFER_SIZE*2];

unsigned long fft_tm;
int16_t Result;
uint32_t Index;
uint32_t i;

void setup() 
{
  Serial.begin(115200);
  delay(10000);
  
  status = ARM_MATH_SUCCESS;
  
  status = arm_rfft_init_q15(&varInstRfftQ15, FFT_SIZE, 0, 1);
  Serial.println("Init FFT Done!"); Serial.flush();
  
  if ( status != ARM_MATH_SUCCESS)
  {
    Serial.print("ERROR: arm_rfft_init_q15 return :"); Serial.flush();
    Serial.println(status); Serial.flush();
    while(1);
  } else {
    Serial.println("Init status: SUCCESS!"); Serial.flush();
  }
  
  for( i=0; i<WAVE_XPICO_SIZE; i+=(BUFFER_SIZE/2))
  {
    /* Copio il segnale ed applico la Hanning window */
    Serial.println("Wave copy and Hanning window"); Serial.flush();
    arm_mult_q15(wave_xpico+i, hanning_int16, samples_q15, BUFFER_SIZE);

    if ( i==(3*(BUFFER_SIZE/2)) )
    {
      Serial.print("wave=[");Serial.flush();
      for( int w=0; w<BUFFER_SIZE;w++)
      {
        Serial.print(samples_q15[w]); Serial.print(",");Serial.flush();
      }
      Serial.println("];");Serial.flush();
    }
    
    Serial.print("Sample Chunk:"); Serial.flush();
    Serial.println(i); Serial.flush();
    
    unsigned long start = millis();
    
    arm_rfft_q15(&varInstRfftQ15, samples_q15, cmplx_q15);
    Serial.println("arm_rfft_q15 Done!"); Serial.flush();
    arm_cmplx_mag_q15(cmplx_q15, bins_q15, BUFFER_SIZE);
    Serial.println("arm_cmplx_mag_q15 Done!"); Serial.flush();
    arm_max_q15(bins_q15, BUFFER_SIZE/2, &Result, &Index);
    Serial.println("arm_max_q15 Done!"); Serial.flush();
    
    fft_tm = millis() - start;
    Serial.print(fft_tm); Serial.flush();
    Serial.println(""); Serial.flush();
    
    for(int ii=0; ii<BUFFER_SIZE/2; ii++)
    {
      Serial.print(bins_q15[ii]);
      Serial.print(",");
    }
    Serial.println("");

    arm_fill_q15( 0, bins_q15, BUFFER_SIZE*2);
    arm_fill_q15( 0, cmplx_q15, BUFFER_SIZE*2);
    
  }  
}


void loop() 
{
  
}
