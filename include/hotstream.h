#include "pciedma.h"
#include "data_patterns.h"

// Core Management

int HotStream_readBin(char *fname, void *idata_buf, int *buf_size);

int HotStream_copyInstrData(int CoreNum, void *idata0, int idata0_size, void *idata1, int idata1_size);

void HotStream_setIntVector(unsigned long intv);

void HotStream_resetCore(int CoreNum);

void HotStream_resetCoreVector(unsigned long rstv);

unsigned long HotStream_getInt();

void HotStream_ackInt(int CoreNum);

void HotStream_ackIntVector(unsigned long rstv);

// Framework Management

int HotStream_init();

int HotStream_close();

// Data Management

int HotStream_sendShMem(void *uBuf, unsigned int size, unsigned int offset);

int HotStream_recvShMem(void *uBuf, unsigned int size, unsigned int offset);

int HotStream_sendBplane(void *uBuf, unsigned int size, int CoreNum);

int HotStream_recvBplane(void *uBuf, unsigned int size, int CoreNum);

int HotStream_startSend(int waitInt);

int HotStream_startRecv(int waitInt);

int HotStream_checkSend();

int HotStream_checkRecv();

// Pattern Definition

int HotStream_linear_send(unsigned int offset, unsigned int size);

int HotStream_linear_recv(unsigned int offset, unsigned int size);

int HotStream_2d_send(int offset, int hsize, int stride, int vsize);

int HotStream_2d_recv(int offset, int hsize, int stride, int vsize);

int HotStream_block_send(int bsize, int mat_size, int elem_size);

int HotStream_block_recv(int bsize, int mat_size, int elem_size);

int HotStream_gather(void *uBuf, void * new_uBuf, int *buf_size, unsigned int offset, unsigned int hsize, unsigned int stride, unsigned int vsize);
