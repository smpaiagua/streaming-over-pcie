#ifndef _PATTERNS_H_
#define _PATTERNS_H_

typedef struct sg_pat{
  unsigned int addr;
  unsigned int hsize;
  unsigned int vsize;
  unsigned int stride;
  struct sg_pat *next;
} sgentry_pattern;

typedef struct {
  unsigned long vma;
  unsigned long size;
  int handle_id;
  int nents;
  sgentry_pattern *sg;
  pd_device_t *pci_handle;
} pd_umem_pattern;

/* sglist_new - Creates a new stack of scatter-gather descriptors for describing a pattern
* Parameters: 
* Returns: A pointer to the head of the stack
*/
sgentry_pattern *sglist_new();

/* sglist_push - Pushes a new descriptor into the end of the stack
* Parameters: base - a pointer to the head of the stack
* 	      addr - base address of the descriptor to insert
* 	      hsize - size of the contiguous block described
* 	      vsize - number of times the contiguous block is to be repeated
* 	      stride - next contiguous block is fetched stride bytes after the starting position of the previous block
* Returns: 0 on success, -1 otherwise
*/
int sglist_push(sgentry_pattern *base, unsigned int addr, unsigned int hsize, unsigned int vsize, unsigned int stride);

/* sglist_pop - Pops a descriptor from the beginning of the stack
* Parameters: base - a pointer to the head of the stack
* Returns: A pointer to a descriptor element
*/
sgentry_pattern *sglist_pop(sgentry_pattern *base);

#endif
