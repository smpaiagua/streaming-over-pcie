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

 if(argc < 2){
    printf("Usage: %s buffer_Size\n",argv[0]);
    return -1;
 }
 
 if(pd_open(0,&dev) != 0){
    printf("Failed to open file \n");
    return -1;
  }

  printf("Device file opened successfuly\n");
  
  
 if(initDMA(&dev) < 0)
   printf("Init DMA failed\n");
 
if(atoi(argv[1]) == 1)
{
 mask = 32;	// reset 5
 genReset(mask);
 return 0;
}
 
     // Test reset
 mask = 0x1F; // reset 0-4
 printf("Activating reset\n");
 
gettimeofday(&tv1,NULL);
genReset(mask);

  
 
// Read Interrupt
  while(1)
  {
    interrupt = getInterrupt();
   // printf("Interrupts: %08x\n", interrupt);
    if(interrupt != 0)
      break;
  }

  gettimeofday(&tv2,NULL);
  curtime = tv2.tv_usec - tv1.tv_usec;
  printf("Time elapsed for 4 lines: %d\n",curtime);
   
  return;


  // Allocate send and receive buffers
 
 size = 8*1024*1024;	// 4096x4096 matrix with 2 byte entries sent in 4 parts
 //size = atoi(argv[1]);
 printf("Buffer size: %d bytes\n",size); 
 
  posix_memalign( (void**)&memr, 16, size);
 
  posix_memalign( (void**)&memA, 16, size);
  posix_memalign( (void**)&memB, 16, size);

 for(i=0;i < size/4; i++){
  ((int*)memA)[i] = 1;
  ((int*)memB)[i] = 1;
 }

 //printf("Random test val: %d\n",((char*)memB)[16777215]);

//--------------------------
 

 // Send 4 parts of each matrix twice
 base_addr = 0x00000000;
 for(i=0; i< 8; i++)
 {
      // Setup copy from the host memory to the MM2S interface
	if(setupSend(&dev, (void*)memA,size,0)<0){
	    printf("Setup Send failed\n");
	    return -1;
	}

      // Setup copy from the S2MM interface to the DDR
	if(setupRecvtoDDR(&dev, base_addr,size)<0){
	    printf("Setup Recv to DDR failed\n");
	    return -1;
	}
	
	// If buffer is bigger than 32768 rely on VSIZE
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
      
	// Start DMA engine. Start by enabling the S2MM so that it blocks waiting for the MM2S 
	if(startRecv(&dev, 0) < 0)
	printf("DMA Recv failed\n");
	
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
	
	freeSend(&dev);
	
	
	// No need to unmap because no user buffer was used
	// freeRecv(&dev);
	
	// Next 4 MB part
	base_addr += 0x800000;
 }
 
// Activate Matrix Multiplication
   
   // Test reset
 mask = 0xFFFFFFFF; // reset all cores
 printf("Activating reset\n");
 //genReset(mask);
 
 
// Read Interrupt
  while(1)
  {
    interrupt = getInterrupt();
    printf("Interrupts: %08x\n", interrupt);
    if(interrupt != 0)
      break;
  }
   
   
  

  
  
 
  stopDMA(&dev);
  
  free(memA); free(memB);
      

  return 0;
}
