#include "hotstream.h"
#include <stdio.h>

pd_device_t dev; // PCIExpress based device handler
int patternApplied = 0; // Boolean flag to indicate whether a pattern has been applied to a memory buffer
int sendTransfType = 0; // Flag to indicate the type of send transfer being initiated (0 - Host to DDR; 1 - Host to Backplane; 2 - Host to Instruction Stream
int recvTransfType = 0; // Flag to indicate the type of recv transfer being initiated (0 - DDR to Host; 1 - Backplane to Host; 2 - Instruction Stream to Host

// Core Management

/* Reads a binary file produced by the Micro16 assembler into a buffer pointed to by idata buf
* fname - Name of the binary output by the Micro16 assembler
* idata_buf - Pointer to the buffer that will hold the instruction data
* buf_size - Pointer to buffer size
* Returns: 0 on success; -1 otherwise
*/
int HotStream_readBin(char *fname, void *idata_buf, int *buf_size)
{
 FILE *finstr;
 char line[10];
 int instrcount = 0,i; 
 short test;

 PRINT("Opening binary file %s\n",fname);

 finstr = fopen(fname, "r");
 if (finstr == NULL){
   PRINT("Could not open binary file %s\n",fname);
   return -1;
 }

 while(fgets(line, 10, finstr) != NULL)
 {
   instrcount++;
 }

 *buf_size = instrcount*2;
 PRINT("Allocating a %d bytes buffer\n", *buf_size);

 if(posix_memalign((void**)&idata_buf, 16, instrcount*2) != 0)
  return -1;

 // Rewind file
rewind(finstr);

 i = 0;
 while(fgets(line, 10, finstr) != NULL)
 {
   sscanf(line, "%04X",&((short*)idata_buf)[i]);
   i++;
 }

 fclose(finstr); 

 return 0;
}

int HotStream_copyInstrData(int CoreNum, void *idata0, int idata0_size, void *idata1, int idata1_size)
{
 int DSSdest;  
 int bar0, bar1;
 unsigned int ICR_data = 0;

   // Configure DataStreamSwitch(DSS)
  DSSdest = 2; // 10 - Shared Memory
  DSSdest = DSSdest << 4; 
  cmuWrite(CMU_KCR, (DSSdest & CMU_KCR_Dest));
 
 bar0 = 0; // Starting position of first instruction block

 bar1 = bar0 + idata0_size/2; // Second instruction block starts right after the first

 if(bar1 + idata1_size/2 > 1024){
   PRINT("Total instruction size for core %d exceeds local memory capacity\n",CoreNum);
   return -1;
 }

 ICR_data = ICR_data | ((bar0 << 4) & CMU_ICR_IBA); // Set BAR0
 ICR_data = ICR_data | ((0 << 1) & CMU_ICR_IO); // Select first partition (0)
 ICR_data = ICR_data | ((CoreNum << 16) & CMU_ICR_CS); // Select Instruction Block to configure
 ICR_data = ICR_data | (1 & CMU_ICR_RS); // Start instruction streaming

 // Copy buffer 0 to the Instruction Stream
//-------------------------------------------

   // Configure DataStreamSwitch(DSS)
  DSSdest = 2; // 10 - Instruction Stream
  DSSdest = DSSdest << 4; 
  cmuWrite(CMU_KCR, (DSSdest & CMU_KCR_Dest));
  
  // Setup data copy from the host memory to the MM2S interface (stream target - CoreNum)
  if(setupSend(&dev, idata0, idata0_size,CoreNum) < 0){
    PRINT("Could not setup a data copy from the Host to the MM2S interface and Instruction Stream\n");
    return -1;
  } 
 
  sendTransfType = 1;

  
  if(HotStream_linear_send(0, idata0_size) < 0){
    PRINT("Could not apply linear pattern to the instruction stream\n");
    return -1;
  }

 // Start send and wait on interrupt 
 if(HotStream_startSend(1) < 0){
   PRINT("Could not stream instruction data to the selected core\n");
   return -1;
 }

 // Verify send transfer and terminate memory mapping
 if(HotStream_checkSend(1) < 0){
   PRINT("Instruction streaming was not successful\n");
   return -1;
 }
 
 ICR_data = ICR_data | ((bar1 << 4) & CMU_ICR_IBA); // Set BAR1
 ICR_data = ICR_data | ((1 << 1) & CMU_ICR_IO); // Select first partition (1)
 ICR_data = ICR_data | ((CoreNum << 16) & CMU_ICR_CS); // Select Instruction Block to configure
 ICR_data = ICR_data | (1 & CMU_ICR_RS); // Start instruction streaming

// Copy buffer 1 to the Instruction Stream

  // Setup data copy from the host memory to the MM2S interface (stream target - CoreNum)
  if(setupSend(&dev, idata1, idata1_size,CoreNum) < 0){
    PRINT("Could not setup a data copy from the Host to the MM2S interface and Instruction Stream\n");
    return -1;
  } 
 
  sendTransfType = 1;

  
  if(HotStream_linear_send(0, idata1_size) < 0){
    PRINT("Could not apply linear pattern to the instruction stream\n");
    return -1;
  }

 // Start send and wait on interrupt 
 if(HotStream_startSend(1) < 0){
   PRINT("Could not stream instruction data to the selected core\n");
   return -1;
 }

 // Verify send transfer and terminate memory mapping
 if(HotStream_checkSend(1) < 0){
   PRINT("Instruction streaming was not successful\n");
   return -1;
 }
 

  return 0;

}

void HotStream_setIntVector(unsigned long intv)
{
 unsigned int mask32 = 0xFFFF;

 cmuWrite(CMU_KCML,(unsigned int)(intv & mask32));
 cmuWrite(CMU_KCMH,(unsigned int)((intv << 32) & mask32));
 
 cmuWrite(CMU_KCR, CMU_KCR_SetInt); // Apply interrupt vector
}

void HotStream_resetCore(int CoreNum)
{
 int bit = 1;
 unsigned int mask = 0;

 mask = mask | (bit << CoreNum);

 genReset(mask);
 
}

void HotStream_resetCoreVector(unsigned long rstv)
{
 genReset(rstv);
}

unsigned long HotStream_getInt()
{
  unsigned long retval;

  retval = cmuRead(CMU_KIVL);
  retval = retval | ((unsigned long)cmuRead(CMU_KIVH))<<32;

  return retval;  
}

void HotStream_ackInt(int CoreNum)
{
  int bit = 1;
  unsigned long intMask64 = 0;
  unsigned int mask32 = 0xFFFF;

  intMask64 = intMask64 | (bit << CoreNum);

  cmuWrite(CMU_KIVL, (unsigned int)(intMask64 & mask32));
  cmuWrite(CMU_KIVH, (unsigned int)((intMask64 << 32) & mask32));
}

void HotStream_ackIntVector(unsigned long rstv)
{
  unsigned int mask32 = 0xFFFF;

  cmuWrite(CMU_KIVL, (unsigned int)(rstv & mask32));
  cmuWrite(CMU_KIVH, (unsigned int)((rstv << 32) & mask32));
}

// Framework Management

int HotStream_init()
{
  if(pd_open(0,&dev) != 0){
   PRINT("Failed to open file \n");
   return -1;
  }

  PRINT("Device file opened successfully\n");

  if(initDMA(&dev) < 0) 
    PRINT("Could not initialize DMA engine\n");

  return 0;
}

int HotStream_close()
{
  if(stopDMA(&dev)<0)
   return -1;

  return pd_close(&dev);
}

// Data Management

int HotStream_sendShMem(void *uBuf, unsigned int size, unsigned int offset)
{
  int DSSdest;

  // Configure DataStreamSwitch(DSS)
  DSSdest = 0; // 00 - Shared Memory
  DSSdest = DSSdest << 4; 
  cmuWrite(CMU_KCR, (DSSdest & CMU_KCR_Dest));
  

  // Setup data copy from the host memory to the MM2S interface (stream target - 0)
  if(setupSend(&dev, uBuf, size,0) < 0){
    PRINT("Could not setup a data copy from the Host to the MM2S interface\n");
    return -1;
  }

  // Setup data copy from the S2MM interface to the Shared Memory (DDR)
  if(setupRecvtoDDR(&dev, offset, size) < 0){
    PRINT("Could not setup a data copy from the S2MM interface to the Shared Memory\n");
    return -1;
  }

  sendTransfType = 0;

  return 0;
}

int HotStream_recvShMem(void *uBuf, unsigned int size, unsigned int offset)
{
  int DSSdest;  

   // Configure DataStreamSwitch(DSS)
  DSSdest = 0; // 00 - Shared Memory
  DSSdest = DSSdest << 4; 
  cmuWrite(CMU_KCR, (DSSdest & CMU_KCR_Dest));
  
  // Setup data transfer from the Shared Memory to the MM2S interface
  if(setupSendfromDDR(&dev, offset, size, 0) < 0){
    PRINT("Could not setup a data copy from the Shared Memory to the MM2S interface\n");
    return -1;
  }

  // Setup data transfer from the S2MM interface to the Host memory
  if(setupRecv(&dev, uBuf, size) < 0){
    PRINT("Could not setup a data copy from the S2MM interface to the Host memory\n");
    return -1;
  }

  recvTransfType = 0;

  return 0;
}

int HotStream_sendBplane(void *uBuf, unsigned int size, int CoreNum)
{
  int DSSdest;  

   // Configure DataStreamSwitch(DSS)
  DSSdest = 1; // 01 - Shared Memory
  DSSdest = DSSdest << 4; 
  cmuWrite(CMU_KCR, (DSSdest & CMU_KCR_Dest));
  
  // Setup data copy from the host memory to the MM2S interface (stream target - CoreNum)
  if(setupSend(&dev, uBuf, size,CoreNum) < 0){
    PRINT("Could not setup a data copy from the Host to the MM2S interface and backplane\n");
    return -1;
  } 
 
  sendTransfType = 1;

  return 0;
}

int HotStream_recvBplane(void *uBuf, unsigned int size, int CoreNum)
{
  int DSSdest;  

   // Configure DataStreamSwitch(DSS)
  DSSdest = 1; // 01 - Shared Memory
  DSSdest = DSSdest << 4; 
  cmuWrite(CMU_KCR, (DSSdest & CMU_KCR_Dest));
  
  // Setup data copy from the S2MM interface to the Host memory (stream target - CoreNum)
  if(setupRecv(&dev, uBuf, size) < 0){
    PRINT("Could not setup a data copy from the S2MM interface to the Host memory\n");
    return -1;
  } 

  recvTransfType = 1;
 
  return 0;
}

int HotStream_startSend(int waitInt)
{
 
 if (!patternApplied){
  PRINT("An access pattern must be applied to the memory buffer before it can be streamed to the HotStream platform\n");
  return -1;
 }
 
 if(sendTransfType == 0){ // Host to Shared Memory
   // Start DMA engine. Start by enabling the S2MM so that it blocks waiting for the MM2S 
   if(startRecv(&dev, waitInt) < 0){
     PRINT("DMA Recv failed\n");
     return -1;
   }
 }
 
 // Now for the MM2S...
 if(startSend(&dev, waitInt) < 0){
   PRINT("DMA Send failed\n");
   return -1;
 }
  
 return 0;
}

int HotStream_startRecv(int waitInt)
{
 if (!patternApplied){
  PRINT("An access pattern must be applied to the memory buffer before it can be streamed from the HotStream platform\n");
  return -1;
 }
 
 if(recvTransfType == 0){ // Shared Memory to Host
   // Start DMA engine. Start MM2S channel 
   if(startSend(&dev, waitInt) < 0){
     PRINT("DMA Recv failed\n");
     return -1;
   }
 }
 
 // Now for the S2MM...
 if(startRecv(&dev, waitInt) < 0){
   PRINT("DMA Send failed\n");
   return -1;
 }
  
 return 0;
 
}

int HotStream_checkSend(int closeMapping)
{
 if(sendTransfType == 0){ // Host to Shared Memory
   if(checkRecvNoBuf() < 0){
     PRINT("Could not verify transfer from S2MM interface to the Shared Memory\n");
     return -1;
    }
 }

 if(checkSend() < 0){
   PRINT("Could not verify transfer from Host to MM2S interface\n");
   return -1;
 }

 if(closeMapping){ // Terminate current mapping to bus device space
  return freeSend(&dev);
 }

 return 0;
}

int HotStream_checkRecv(int closeMapping)
{
 if(recvTransfType == 0){ // Shared Memory to Host
   if(checkSend() < 0){
     PRINT("Could not verify transfer from Shared Memory to MM2S interface\n");
     return -1;
   } 
 }

 if(checkRecv() < 0){
     PRINT("Could not verify transfer from S2MM interface to the Hosty\n");
     return -1;
    }

 if(closeMapping){
   return freeRecv(&dev);
 }

 return 0;
}

// Pattern Definition

int HotStream_linear_send(unsigned int offset, unsigned int size)
{
  if(sendTransfType == 0){
   // Apply Linear pattern to the data copied to the Shared Memory
   if(applyLinear_recv(offset,size,size,size) < 0){
     PRINT("Could not apply linear pattern\n");
     return -1;
   }
  } 

  if(applyLinear_send(offset,size,size,size) < 0){
    PRINT("Could not apply linear pattern\n");
    return -1;
  }

  return 0;
}

int HotStream_linear_recv(unsigned int offset, unsigned int size)
{
  if(recvTransfType == 0){
    // Apply Linear pattern to the data retrieved from the Shared Memory
    if(applyLinear_send(offset,size,size,size) < 0){
      PRINT("Could not apply linear pattern\n");
      return -1;
    }
   } 

   if(applyLinear_recv(offset,size,size,size) < 0){
     PRINT("Could not apply linear pattern\n");
     return -1;
   }

   return 0;
}

int HotStream_2d_send(int offset, int hsize, int stride, int vsize)
{
  if(sendTransfType == 0){
    // Apply Linear pattern to the data copied to the Shared Memory
    if(apply2d_recv(offset,hsize,stride,vsize) < 0){
      PRINT("Could not apply 2D pattenn\n");
      return -1;
    }
   } 

   if(apply2d_send(offset,hsize,stride,vsize) < 0){
     PRINT("Could not apply 2D pattern\n");
     return -1;
   }

   return 0;
}


int HotStream_2d_recv(int offset, int hsize, int stride, int vsize)
{
  if(recvTransfType == 0){
     // Apply Linear pattern to the data retrieved from the Shared Memory
     if(apply2d_send(offset,hsize,stride,vsize) < 0){
       PRINT("Could not apply 2D pattern\n");
       return -1;
     }
    } 

    if(apply2d_recv(offset,hsize,stride,vsize) < 0){
      PRINT("Could not apply 2D pattern\n");
      return -1;
    }

    return 0;
}

int HotStream_block_send(int bsize, int mat_size, int elem_size)
{

}

int HotStream_block_recv(int bsize, int mat_size, int elem_size)
{
  if(recvTransfType == 0){
     // Apply Linear pattern to the data retrieved from the Shared Memory
     if(applyBlocking_send(bsize, mat_size, elem_size) < 0){
       PRINT("Could not apply blocking pattern\n");
       return -1;
     }
    } 

    if(applyBlocking_recv(bsize, mat_size, elem_size) < 0){
      PRINT("Could not apply blocking pattern\n");
      return -1;
    }

    return 0;
}

int HotStream_gather(void *ubuf, void *new_ubuf, int *buf_size, unsigned int offset, unsigned int hsize, unsigned int stride, unsigned int vsize)
{
 return compressData(ubuf, new_ubuf, buf_size, offset, hsize, stride, vsize);
}

int main(int argc, char **argv)
{
 void *idata0;
 int size;

 HotStream_readBin(argv[1], idata0, &size);

 return 0;
}
