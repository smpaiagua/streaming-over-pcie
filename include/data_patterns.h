#ifndef _DATA_PATTERNS_H_
#define _DATA_PATTERNS_H_

int apply2d_send(int offset, int hsize, int stride, int vsize);

int apply2d_recv(int offset, int hsize, int stride, int vsize);

int applyBlocking_send(int bsize, int mat_size, int elem_size);

int applyBlocking_recv(int bsize, int mat_size, int elem_size);

int applyLinear_send(int offset, int hsize, int stride, int total_size);

int applyLinear_recv(int offset, int hsize, int stride, int total_size);


/* compressData - Takes a 2D pattern and transforms it into a stream-ready block by allocating a new buffer. Pattern must fit within ubuf 
* Parameters: ubuf - pointer to original user buffer
*	      new_buf - pointer to the newly allocated user buffer
*	      buf_size - will hold the size of the newly allocated buffer
*	      hsize - HSIZE of the pattern
*	      stride - STRIDE of the pattern
*	      vsize - VSIZE of the pattern
* Returns : 0 if successful, -1 otherwise
*/
int compressData(void *ubuf, void *new_ubuf, int *buf_size, unsigned int offset, unsigned int hsize, unsigned int stride, unsigned int vsize);

#endif
