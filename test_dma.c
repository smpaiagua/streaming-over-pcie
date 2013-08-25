#include "pciedma.h"
#include "data_patterns.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define SEND

int main(int argc, char **argv)
{
  unsigned int *data_samples;
  int i,val,j;
  int size, new_size;
  pd_device_t dev;
  void *memr, *memA, *memB, *mems, *mem_new;
  float throughput;
  struct timeval tv1,tv2;
  int mintime, curtime, vsize;
  
  unsigned int mask, interrupt, base_addr;
  int div_factor = 1;
  
 printf("====================================\n");
 printf("=       Pattern Tests	            =\n");
 printf("====================================\n\n");

 
 if(pd_open(0,&dev) != 0){
    printf("Failed to open file \n");
    return -1;
  }

  printf("Device file opened successfuly\n");
  
  
 if(initDMA(&dev) < 0)
   printf("Init DMA failed\n");
 


  // Allocate send and receive buffers
 
 size = 9216;
 //size = 8388608;;
   
 printf("Buffer size: %d bytes\n",size); 
 
  posix_memalign( (void**)&mems, 16, size);
 
  //posix_memalign( (void**)&memr, 16, size);

  
gettimeofday(&tv1,NULL); 

      // Setup copy from the host memory to the MM2S interface
	if(setupSend(&dev, (void*)mems,size,0)<0){
	    printf("Setup Send failed\n");
	    return -1;
	}

      // Setup copy from the S2MM interface to the DDR
	if(setupRecvtoDDR(&dev, base_addr,size)<0){
	    printf("Setup Recv to DDR failed\n");
	    return -1;
	}


	if(size > 32768)
          div_factor = size/32768;
    
        // Apply Linear pattern to the Send data
        if( applyLinear_send(0, size/div_factor, size/div_factor, size) < 0){ 
          printf("Apply linear send failed\n");
          return -1; 
        }   
    
        // Apply Linear pattern to the Recv data
        if(applyLinear_recv(0,size/div_factor, size/div_factor,size) < 0){  
          printf("Apply Linear recv failed\n");
          return -1; 
        }   


gettimeofday(&tv2,NULL); 


	curtime = tv2.tv_usec - tv1.tv_usec;
	printf("Time taken to setup everything : %d\n",curtime);

    
        // Start DMA engine. Start by enabling the S2MM so that it blocks waiting for the MM2S 
        if(startRecv(&dev, 0) < 0)
        printf("DMA Recv failed\n");

	
	gettimeofday(&tv1,NULL);
	    
        
	// Now for the MM2S...
        if(startSend(&dev, 0) < 0)
        printf("DMA Send failed\n");
	
	// Check completion of the transfers
	
	// Recv does not have a buffer associated
	printf("Checking recv...\n");
	if(checkRecvNoBuf() < 0)
	  printf("Check DMA recv failed\n");

	printf("Checking send...\n");
	if(checkSend() < 0)
	  printf("Check DMA send failed\n");
	
	gettimeofday(&tv2,NULL);

	curtime = tv2.tv_usec - tv1.tv_usec;
	printf("Time elapsed : %d\n",curtime);

	freeSend(&dev);
	
	

 
  stopDMA(&dev);
  
  free(mems); //free(memr);
      

  return 0;
}
