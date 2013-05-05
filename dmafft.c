#include "pciedma.h"
#include "fftcontrol.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#define NSAMPLES	1024
#define PI 		3.14159265

#define MASK16		0xFFFF

unsigned int* create_signal()
{
 int i;
 double theta, theta2, re_real, im_real;
 unsigned int re_int, im_int;
 unsigned int* data_samples;
 
 void *data;
 
 data_samples = (unsigned int*)malloc(sizeof(unsigned int)*NSAMPLES);
 if(data_samples == NULL){
    printf("Error: Failed to allocate data samples buffer\n");
    return NULL;
 }
 
 posix_memalign( (void**)&data, 16, sizeof(unsigned int)*NSAMPLES );
 
 for(i=0; i < NSAMPLES; i++)
 {
    theta = ((double)i/(double)NSAMPLES)*2.6*2.0 * PI;
    re_real = cos(-theta);
    im_real = sin(-theta);
    theta2 = ((double)i/(double)NSAMPLES)*23.2*2.0 * PI;
    re_real = re_real + (cos(-theta2) / 4.0);
    im_real = im_real + (sin(-theta2) / 4.0);
    re_int = (unsigned int)nearbyint(re_real * pow(2,14));
    im_int = (unsigned int)nearbyint(im_real * pow(2,14));
    im_int = im_int & MASK16;
    im_int = im_int << 16;
    
    ((unsigned int*)data)[i] = 0;
    ((unsigned int*)data)[i] = re_int;
    ((unsigned int*)data)[i] = ((unsigned int*)data)[i] & MASK16 | (im_int);
 }
 
 return data;
}

int main()
{
  unsigned int *data_samples;
  int i;
  pd_device_t dev;
  void *memr;
  
 printf("FFT computation demonstration\n");
 
 if(pd_open(0,&dev) != 0){
    printf("Failed to open file \n");
    return -1;
  }


  // Create block of 1kB memory
  posix_memalign( (void**)&memr, 16, sizeof(unsigned int)*NSAMPLES);
 
 data_samples = create_signal();
 
 
// for(i=0;i<64;i++)
//   printf("%d: %08x\n",i,data_samples[i]);
 
 if(initDMA(&dev) < 0)
   printf("Init DMA failed\n");
 
 FFTreset();
 //configFFT(10,0x2AA,0);
 
// if(setFFTfwdinv(1) < 0)
//   printf("Could not set FFT direction\n");
 
// if(setFFTscaling(0x2AA) < 0)
//   printf("Could not set FFT scaling\n");
 
   
// if(setFFTsize(10) < 0)
//   printf("Could not set FFT size\n");
   
//   printf("FFT Status: %08x\n",readFFTstatus());

    
 
 if(setupSend(&dev, (void*)data_samples, sizeof(unsigned int)*NSAMPLES,3) < 0)
   printf("Setup Send failed\n");
 
 if(startSend(&dev, 0) < 0)
   printf("DMA Send failed\n");

 if(checkSend() < 0)
   printf("Check DMA send failed\n");
 


 freeSend(&dev);

 // Receive result
 
 setupRecv(&dev, memr, sizeof(unsigned int)*NSAMPLES);

 startRecv(&dev, 1);

 checkRecv();
 
 freeRecv(&dev);

 stopDMA(&dev);

 
 for(i=0;i<64;i++)
  // printf("%d: y_re =  %d, y_im = %d\n",i,(short)(((int*)memr)[i] & MASK16), (short)(((int*)memr)[i] >> 16));
  printf("%d: %08x\n",i,((unsigned int*)memr)[i]);

 
  
  return 0;
}
