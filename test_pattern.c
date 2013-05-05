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
  int i,val;
  int size, new_size;
  pd_device_t dev;
  void *memr, *mems, *mem_new;
  float throughput;
  struct timeval tv1,tv2;
  int mintime, curtime;

 printf("====================================\n");
 printf("=       Pattern Tests	            =\n");
 printf("====================================\n\n");

 if(pd_open(0,&dev) != 0){
    printf("Failed to open file \n");
    return -1;
  }

  printf("Device file opened successfuly\n");

  // Allocate send and receive buffers
 
  size = 4*32*32; // 32x32 matrix with 4 byte entries
 
  posix_memalign( (void**)&memr, 16, size);
  posix_memalign( (void**)&mems, 16, size);

 for(i=0;i < size/4; i++){
  ((int*)mems)[i] = i;
  printf("%d ",((int*)mems)[i]);
 }

 compressData(mems, &mem_new, &new_size, 4, 40, 80, 10);

 for(i=0;i < new_size/4; i++){
  printf("%d ",((int*)mem_new)[i]);
 }

 return;

  // Each 5x5 subblock will hold the same value
  val = 0;
  for(i=0; i < size/4; i++){
    if (i % 16 == 0 && i!=0)
 	val ++;

    val = val % 2;

    ((int *)mems)[i] = val;
  }

  // Test matrix
  for(i=0; i < size/4; i++){
    if (i%32 == 0 && i!=0)
     printf("\n");
  
    printf("%d ", ((int*)mems)[i]);
    
  } 


//--------------------------
 
 if(initDMA(&dev) < 0)
   printf("Init DMA failed\n");
 
 // Send and receive in simultaneous
  
gettimeofday(&tv1,NULL);  

#ifdef SEND
if(setupSend(&dev, (void*)mems, size,1) < 0)
  printf("Setup Send failed\n");

	if(applyBlocking_send(16, 32, 4) < 0)
	 printf("Apply Blocking send failed\n");
#endif
 if(setupRecv(&dev, (void*)memr, size) < 0)
   printf("Setup Recv failed\n");


//int applyLinear_recv(int offset, int hsize, int stride, int total_size);
//	if(applyBlocking_recv(16, 32, 4) < 0)
	if(applyLinear_recv(0,128,128,4096) < 0)  
 	 printf("Apply Blocking recv failed\n");


  gettimeofday(&tv2,NULL);  

  printf("Time taken to setup receive AND send transfer:%d\n", tv2.tv_usec - tv1.tv_usec);

  //--------------------------

  gettimeofday(&tv1,NULL);  
#ifdef SEND
 //if(startSend(&dev, 0) < 0)
 //  printf("DMA Send failed\n");
#endif
 // printf("Press any key to continue ;)\n");
 // getchar();

  if(startRecv(&dev, 0) < 0)
   printf("DMA Recv failed\n");
 
  if(startSend(&dev, 0) < 0)
   printf("DMA Send failed\n");
   

   if(checkRecv() < 0)
    printf("Check DMA recv failed\n");
#ifdef SEND
   if(checkSend() < 0)
    printf("Check DMA send failed\n");
#endif

  
  
  gettimeofday(&tv2,NULL);  
  
   curtime = tv2.tv_usec - tv1.tv_usec;
   printf("Time taken to complete and check send AND receive transfer:%d\n", curtime);
   printf("Throughput: %f MB/s\n",(float)2*size/((float)(curtime)));


  // Test matrix
  for(i=0; i < size/4; i++){
    if (i%32 == 0 && i!=0)
     printf("\n");
  
    printf("%d ", ((int*)memr)[i]);
  } 
  
 freeSend(&dev);

 freeRecv(&dev);
 
 stopDMA(&dev);
 
 //for(i=0;i<size/4;i++)
 // printf("%d ",((unsigned int*)memr)[i]);
 
 free(memr); free(mems);

  return 0;
}
