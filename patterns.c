#include "/root/Documents/spaiagua/Swinger/pcie_dev_driver/pciDriver/include/lib/pciDriver.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "patterns.h"
#include "pciedma.h"
#include "desc_mgmt.h"

/* GLOBAL VARIABLES FROM pciedma.h */
pd_umem_t *umem_tr_snd, *umem_tr_recv;


sgentry_pattern *sglist_new()
{
 sgentry_pattern *base;
 
 base = (sgentry_pattern*)malloc(sizeof(sgentry_pattern));
 if (base == NULL){
    PRINT("Error: Could not malloc new list element\n");
    return NULL;
 }
 
 base->next = NULL;
 
 return base;
}

int sglist_push(sgentry_pattern *base, unsigned int addr, unsigned int hsize, unsigned int vsize, unsigned int stride)
{
 sgentry_pattern *next, *new;
 
 next = base;
 
 while(next->next != NULL)
   next = next->next;
 
 // Found insertion point
 new = (sgentry_pattern*)malloc(sizeof(sgentry_pattern));
 if (new == NULL){
    PRINT("Error: Could not malloc new list element\n");
    return -1;
 }
 
 new->addr = addr;
 new->hsize = hsize;
 new->vsize = vsize;
 new->stride = stride;
 new->next = NULL;
 
 next->next = new;
 
 return 0;
}

sgentry_pattern *sglist_pop(sgentry_pattern *base)
{
 sgentry_pattern *next;
 
 next = base;
 
 if (next->next == NULL)
   return NULL; // List is empty
   
 next = next->next;
 base->next = next->next;
 
 return next;
}

int create_pattern_struct(pd_umem_t *umem, pd_umem_pattern **umem_pattern)
{
	
  if (umem->nents == 0){
     PRINT("Error: SG list is empty\n");
     return -1;
  } 

  // Create a umem structure with translated addresses
  *umem_pattern = (pd_umem_pattern*)malloc(sizeof(pd_umem_pattern));
  if(*umem_pattern == NULL){
     PRINT("Error: Could not malloc umem structure\n");
     return(-1);
  }

  (*umem_pattern)->vma = umem->vma;
  (*umem_pattern)->size = umem->size;
  (*umem_pattern)->handle_id = umem->handle_id;
  (*umem_pattern)->pci_handle = umem->pci_handle;
 
  (*umem_pattern)->nents = 0;
  (*umem_pattern)->sg = sglist_new(); // Create list for holding descriptors after patternization

  return 0;
}

int pattern2d(pd_umem_t *umem, pd_umem_pattern **umem_pattern, unsigned int offset, unsigned int hsize, unsigned int stride, unsigned int vsize)
{
  unsigned int T, N, F, desc_size, ndesc_orig, ndesc_final;
  int i;
 
  unsigned int base_addr;
  

  ndesc_orig = umem->nents; // Number of original descriptors resulting from mapping to device space

  ndesc_final = (*umem_pattern)->nents; // Will hold the number of descriptors after patternization


  base_addr =  offset;
  for(i = 0; i < ndesc_orig; i++)
  {
    if(vsize == 0)
     break;	// All blocks have been written

    if (base_addr >= (umem->sg[i]).size)
    {
	// base address does not fit in the current descriptor
	base_addr -= (umem->sg[i]).size;
	continue;
    }

    N = ((umem->sg[i]).size - base_addr) / stride;
    F = ((umem->sg[i]).size - base_addr) % stride;

   if(N >= vsize){
     // Pattern fits into a descriptor space   
      sglist_push((*umem_pattern)->sg,base_addr+(umem->sg[i]).addr,hsize,vsize,stride);
      //offset = 0; // Offset has been included
      ndesc_final++;
      (*umem_pattern)->nents = ndesc_final;
      return 0;
   }

   if (F != 0)
   {
      if (F < hsize)
      {
	// Write previous descriptor
	if (N != 0)
	{
	   sglist_push((*umem_pattern)->sg,base_addr+(umem->sg[i]).addr,hsize,N,stride);
	   ndesc_final++;
	   vsize -= N;
	}	
	
	// Write fraction last block on current descriptor
	base_addr += N*stride;
	sglist_push((*umem_pattern)->sg,base_addr+(umem->sg[i]).addr,F,1,stride);
	ndesc_final++;

	// Write remainder of last block on next descriptor
	base_addr = 0;
	sglist_push((*umem_pattern)->sg,base_addr+(umem->sg[i+1]).addr,hsize-F,1,stride);
	ndesc_final++;
	vsize -= 1;
	
	base_addr = stride - F;
      }
      else if (F >= hsize)
      {
	// Write previous descriptor including the current block
	sglist_push((*umem_pattern)->sg,base_addr+(umem->sg[i]).addr, hsize, N+1,stride);
	ndesc_final++;
	vsize -= (N+1);

	base_addr = stride - F;
      }
   }
  }
 
 (*umem_pattern)->nents = ndesc_final;

 return 0; 
}


/* compressData - Takes a 2D pattern and transforms it into a stream-ready block by allocating a new buffer. Pattern must fit within ubuf 
* Parameters: ubuf - pointer to original user buffer
*	      new_buf - pointer to the newly allocated user buffer
*	      buf_size - will hold the size of the newly allocated buffer
*	      hsize - HSIZE of the pattern
*	      stride - STRIDE of the pattern
*	      vsize - VSIZE of the pattern
* Returns : 0 if successful, -1 otherwise
*/
int compressData(void *ubuf, void **new_ubuf, int *buf_size, unsigned int offset, unsigned int hsize, unsigned int stride, unsigned int vsize)
{
 int new_size;
 int i,j,ctr;

 new_size = hsize*vsize;

 if(posix_memalign((void**)new_ubuf, 16, new_size)!= 0){
   PRINT("Could not allocate new user buffer\n");
   return -1;
 }

 *buf_size = new_size;
 ctr = 0;
for (i=0; i < vsize; i++)
{
  for (j=0;j < hsize/4; j++)
  {
    ((int *)(*new_ubuf))[ctr] = ((int *)ubuf)[offset/4 + i*stride/4 + j];
     ctr++;
  }
}
 
return 0;
}

int pattern2d_old(pd_umem_t *umem, pd_umem_pattern **umem_pattern, unsigned int offset, unsigned int hsize, unsigned int stride, unsigned int vsize)
{
  unsigned int T, N, F, desc_size, ndesc_orig, ndesc_final;
  int i;
  
  unsigned int base_addr, stride_new, hsize_new, vsize_new, transitive_offset;


  ndesc_orig = umem->nents; // Number of original descriptors resulting from mapping to device space
  
  ndesc_final = (*umem_pattern)->nents; // Will hold the number of descriptors after patternization
  
  transitive_offset = 0; // Offset originated from non-integer division of block "periods" over descriptor spaces
  
  // Create a 2D descriptor for N block periods
  hsize_new = hsize;
  stride_new = stride;
  for(i = 0; i < ndesc_orig; i++)
  {
    if(vsize == 0)
      break;		// All block periods are written  

    T = offset + hsize + stride; // Block "period"
    N = ((umem->sg[i]).size - transitive_offset - offset) / T;
    F = ((umem->sg[i]).size - transitive_offset - offset) % T;
    
    base_addr = (umem->sg[i]).addr + transitive_offset + offset;


    if (N >= vsize){
      // Pattern fits into one descriptor space
      sglist_push((*umem_pattern)->sg,base_addr,hsize_new,vsize,stride_new);
      offset = 0; // Offset has been included
      ndesc_final++;
      (*umem_pattern)->nents = ndesc_final;
      return 0;
    }
    
    vsize_new = N;	// Number of block periods
    if (N>0)
	offset = 0; // Offset will be included in first descriptor 
      
    if (F != 0)
    {
	if (F <= offset)
	{
	    transitive_offset += (offset - F);
	}
	else if (F > offset && F <= hsize + offset)
	{
	  if (N > 0)
 	  {
	    // Write largest 2D descriptor
            sglist_push((*umem_pattern)->sg,base_addr,hsize_new,vsize_new,stride_new);
            ndesc_final++;
            vsize -= N;         // Decrease the number of block periods to write
	  }

	  base_addr = (umem->sg[i]).addr + T*N + offset;
	  hsize_new = F - offset;
	  vsize_new = 1;
	  
	  sglist_push((*umem_pattern)->sg,base_addr,hsize_new,vsize_new,stride_new); // Write a fraction of HSIZE to the current descriptor space
	  ndesc_final++;
	  offset = 0;	  

	  base_addr = (umem->sg[i+1]).addr;
	  hsize_new = offset + hsize - F;
	  vsize_new = 1;
	  
	  sglist_push((*umem_pattern)->sg,base_addr,hsize_new,vsize_new,stride_new); // Write a fraction of HSIZE to the next descriptor space
	  ndesc_final++;
	  vsize--;	// One more block period written
	  
	  transitive_offset += stride_new; // Add stride to the original offset
	  
	  vsize -= N;	// Decrease the number of block periods to write
	  
	}
	else if (F > offset + hsize && F <= hsize + stride_new)
	{
	  // Write block as part of the previous largest 2D descriptor
	  vsize_new++;
    	  sglist_push((*umem_pattern)->sg,base_addr,hsize_new,vsize_new,stride_new);
    	  ndesc_final++;
 	  vsize -= vsize_new; 	// Decrease the number of block periods to write 
	  
	  transitive_offset += hsize + stride_new - F;
	}
    } 
  }
  
  (*umem_pattern)->nents = ndesc_final;

  
  return 0;
}

int apply2dpattern(pd_umem_t *umem, pd_umem_pattern **umem_pattern, unsigned int offset, unsigned int hsize, unsigned int stride, unsigned int vsize)
{
  if(create_pattern_struct(umem,umem_pattern)<0)
	return -1;

  pattern2d(umem,umem_pattern,offset,hsize,stride,vsize);
  
  return 0;
}

int apply2d_send(int offset, int hsize, int stride, int vsize)
{
 pd_umem_pattern *umem_pat;

 if (apply2dpattern(umem_tr_snd, &umem_pat, offset, hsize, stride, vsize) < 0){
   return -1;
 }

 if (write_pattern_send(umem_pat) < 0){
   return -1;
 }

 return 0;
}


int apply2d_recv(int offset, int hsize, int stride, int vsize)
{
 pd_umem_pattern *umem_pat;

 if (apply2dpattern(umem_tr_recv, &umem_pat, offset, hsize, stride, vsize) < 0){
   return -1;
 }

 if (write_pattern_recv(umem_pat) < 0){
   return -1;
 }

 return 0;
}


int applyBlocking_send(int bsize, int mat_size, int elem_size)
{
  pd_umem_pattern *umem_pat;

  // Apply blocking pattern to the translated descriptor umem_tr_snd
  if (applyBlocking(umem_tr_snd, &umem_pat, bsize, mat_size, elem_size) < 0){
    return -1;
  }
 
  // Write patternized descriptor into BRAM and setup DMA send
  if (write_pattern_send(umem_pat) < 0){
    return -1;
  }

  return 0;
}


int applyBlocking_recv(int bsize, int mat_size, int elem_size)
{
  pd_umem_pattern *umem_pat;

  // Apply blocking pattern to the translated descriptor umem_tr_recv
  if (applyBlocking(umem_tr_recv, &umem_pat, bsize, mat_size, elem_size) < 0){
    return -1;
  }
 
  // Write patternized descriptor into BRAM and setup DMA send
  if (write_pattern_recv(umem_pat) < 0){
    return -1;
  }

  return 0;
}

/* applyLinear - Applies a linear pattern to a total_size long block. hsize will be repeated an integer number of times 
 * Parameters: umem - pointer to mapped user buffer
 *	       umem_pattern - pointer to hold resulting descriptor list
 *	       offset - the starting position of the pattern, in bytes
 * 	       hsize - the repeated contiguous block, in bytes
 *	       stride - next contiguous block starts stride bytes after the starting position of the previous
 *	       total_size - total size of the block to which to apply the pattern
 */
int applyLinear(pd_umem_t *umem, pd_umem_pattern **umem_pattern, int offset, int hsize, int stride, int total_size)
{
 int vsize = 0;

  if(create_pattern_struct(umem,umem_pattern)<0)
	return -1;

  total_size -= offset;
  
  // Calculates the integer number of repetitions that fit in the specified total_size
  vsize = total_size / stride;
  
  return pattern2d(umem, umem_pattern, offset, hsize, stride, vsize);
}

int applyLinear_send(int offset, int hsize, int stride, int total_size)
{
 pd_umem_pattern *umem_pat;

 if (applyLinear(umem_tr_snd, &umem_pat, offset, hsize, stride, total_size) <0){
   return -1;
 }

 if (write_pattern_send(umem_pat) < 0){
   return -1;
 }

 return 0;
}


int applyLinear_recv(int offset, int hsize, int stride, int total_size)
{
 pd_umem_pattern *umem_pat;

 if (applyLinear(umem_tr_recv, &umem_pat, offset, hsize, stride, total_size) <0){
   return -1;
 }

 if (write_pattern_recv(umem_pat) < 0){
   return -1;
 }

 return 0;
}


/* applyBlocking - Applies a blocking pattern (square matrixes only) to the provided buffer
* Parameters: umem - pointer to mapped user buffer
*	      umem_pattern - pointer to hold resulting descriptor list
*	      bsize - size of the block to use in the blocking pattern (side)
*	      mat_size - total matrix size (side)
*	      elem_size - element size in bytes (4 bytes for integer)
*/
int applyBlocking(pd_umem_t *umem, pd_umem_pattern **umem_pattern, int bsize, int mat_size, int elem_size)
{
  int nblocks, i, j, full_mat_size, full_bsize, bsize_bytes;
  int full_bsize_bytes, block_line;

  if(create_pattern_struct(umem,umem_pattern)<0)
	return -1;

  full_bsize = bsize*bsize;
  full_mat_size = mat_size*mat_size;


  if(full_mat_size%full_bsize != 0){
    PRINT("Error: Matrix size does not fit an integer number of blocks\n");
    return -1;
  }
  nblocks = full_mat_size / full_bsize;  
  PRINT("nblocks: %d\n",nblocks);
  
  
  bsize_bytes = bsize*elem_size; // block size in bytes (column or row)
  full_bsize_bytes = full_bsize*elem_size;
  block_line = mat_size/bsize;	// number of blocks per line
  for(i=0;i< block_line; i++)
   for(j=0;j < block_line; j++)
    pattern2d(umem, umem_pattern, j*bsize_bytes + i*full_bsize_bytes*block_line, bsize_bytes, mat_size*elem_size, bsize); // Repeat 2D pattern for each block within the matrix

  return 0;
}


/* 
int main()
{
 
  if (total_size > (umem->sg[1]).size + (umem->sg[0]).size){
     printf("Pattern (%d) exceedes buffer capacity (%d)!\n",total_size,(umem->sg[1]).size + (umem->sg[0]).size);
     return -1;
  }
  

  printf("Applying 2D pattern to a %d bytes block\n",total_size);
  
  if(apply2dpattern(umem, &umem_pat, offset, hsize, stride, vsize) < 0)
  {
     printf("Could not apply pattern :/\n");
     return -1;
  }
*/
/*
  printf("Applying blocking pattern to a %d bytes block\n",15*15*4);
  
  if(applyBlocking(umem,&umem_pat, 5,15,4)<0)
  {
     printf("Could not apply blocking pattern\n");
     return -1;
  }

  printf("Created %d descriptors.\n",umem_pat->nents);
  descriptor = sglist_pop(umem_pat->sg);
  total_size = 0;
  i=0;
  while(descriptor != NULL)
  {
      printf("%d: ADDR: %08x HSIZE: %d STRIDE: %d VSIZE: %d\n",i,descriptor->addr, descriptor->hsize, descriptor->stride, descriptor->vsize);
      total_size += (descriptor->hsize)*(descriptor->vsize);
      descriptor = sglist_pop(umem_pat->sg);
      i++;
  }

  printf("Descripted size: %d\n",total_size);
  
  
  
}*/
