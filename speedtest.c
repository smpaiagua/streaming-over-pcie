#include "pciedma.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#define NSAMPLES	1024
#define NRUNS		100

int main(int argc, char **argv)
{
  unsigned int *data_samples;
  int i;
  int size;
  pd_device_t dev;
  void *memr, *mems;
  float throughput;
  struct timeval tv1,tv2;
  int mintime, curtime;

 printf("====================================\n");
 printf("=       DMA Throughput Test        =\n");
 printf("====================================\n\n");

 
 if(argc != 2){
    printf("Usage: %s [buffer size]\n",argv[0]);
    return -1;
 }
 
 size = atoi(argv[1]);
 if(size > 524288){
    printf("Wohaa, easy there. Maximum buffer size is 262144\n");
    return -1;
 }
 if(size % 4 != 0){
    printf("Buffer size must be divisible by 4 bytes\n");
    return -1;
 }
 
 printf("Buffer size: %d\n\n",size);
 
 if(pd_open(0,&dev) != 0){
    printf("Failed to open file \n");
    return -1;
  }

  printf("Device file opened successfuly\n");

  // Allocate send and receive buffers
  
  posix_memalign( (void**)&memr, 16, size);
  posix_memalign( (void**)&mems, 16, size);
  
  //for(i=0;i<size/4;i++)
  //  ((unsigned int*)mems)[i] = i;
    
  //for(i=0;i<size/4;i++)
  //  ((unsigned int*)memr)[i] = 9;

 //--------------------------

  
 
 if(initDMA(&dev) < 0)
   printf("Init DMA failed\n");
 
 /*
 gettimeofday(&tv1,NULL);  
 
 if(setupSend(&dev, (void*)mems, size) < 0)
   printf("Setup Send failed\n");
 
 gettimeofday(&tv2,NULL);  

 printf(">> Time taken to setup send transfer:%d\n", tv2.tv_usec - tv1.tv_usec);
 
  //--------------------------
 
 gettimeofday(&tv1,NULL);  

 
 if(setupRecv(&dev, memr, size) < 0)
   printf("Setup Recv failed\n");

 gettimeofday(&tv2,NULL);  

 printf(">> Time taken to setup receive transfer:%d\n", tv2.tv_usec - tv1.tv_usec);
 
 
 //--------------------------
 
 gettimeofday(&tv1,NULL);  
 
 if(startSend(&dev, 0) < 0)
   printf("DMA Send failed\n");

 if(checkSend() < 0)
   printf("Check DMA send failed\n");

  gettimeofday(&tv2,NULL);  
 
 printf(">> Time taken to complete and check send transfer:%d\n", tv2.tv_usec - tv1.tv_usec);
 printf(">> Throughput: %f MB/s\n",(float)size/((float)(tv2.tv_usec - tv1.tv_usec)));
 
  //--------------------------
 
  gettimeofday(&tv1,NULL);  
 
 if(startRecv() < 0)
   printf("DMA Recv failed\n");

 if(checkRecv() < 0)
   printf("Check DMA recv failed\n");

  gettimeofday(&tv2,NULL);  
 
 printf(">> Time taken to complete and check recv transfer:%d\n", tv2.tv_usec - tv1.tv_usec);
  printf(">> Throughput: %f MB/s\n",(float)size/((float)(tv2.tv_usec - tv1.tv_usec)));

 
 */
 
 
   //--------------------------

 // Send and receive in simultaneous
 mintime = 9999;
for(i=0;i<NRUNS;i++){
  gettimeofday(&tv1,NULL);  

 if(setupSend(&dev, (void*)mems, size,1) < 0)
   printf("Setup Send failed\n");
 
 if(setupRecv(&dev, (void*)memr, size) < 0)
   printf("Setup Recv failed\n");

  gettimeofday(&tv2,NULL);  

  printf("Time taken to setup receive AND send transfer:%d\n", tv2.tv_usec - tv1.tv_usec);

  //--------------------------

  gettimeofday(&tv1,NULL);  
  
    if(startRecv(&dev, 0) < 0)
   printf("DMA Recv failed\n");
 
   if(startSend(&dev, 0) < 0)
   printf("DMA Send failed\n");
   

     if(checkRecv() < 0)
   printf("Check DMA recv failed\n");
 
   if(checkSend() < 0)
   printf("Check DMA send failed\n");
  

  
  
  gettimeofday(&tv2,NULL);  
  
   curtime = tv2.tv_usec - tv1.tv_usec;
   printf("Time taken to complete and check send AND receive transfer:%d\n", curtime);
   printf("Throughput: %f MB/s\n",(float)2*size/((float)(curtime)));

  
 freeSend(&dev);

 freeRecv(&dev);
 
 if(curtime < mintime)
   mintime = curtime;
}

 stopDMA(&dev);
 
 //for(i=0;i<size/4;i++)
 // printf("%d ",((unsigned int*)memr)[i]);
 
 free(memr); free(mems);

printf("Maximum throughput for %d points: %f MB/s\n",size,(float)2*size/((float)(mintime)));

 
  
  return 0;
}
