#ifndef _DESC_MGMT_H_
#define _DESC_MGMT_H_

#include "patterns.h"

/* Writes the descriptors resulting from patternization into the BRAM and configures the DMA receive transfer
* Arguments: umem_pat - pointer to structure containing the descriptors resulting from patternization
* Returns: 0 on success, -1 otherwise
*/
int write_pattern_recv(pd_umem_pattern *umem_pat);

/* Writes the descriptors resulting from patternization into the BRAM and configures the DMA send transfer
* Arguments: umem_pat - pointer to structure containing the descriptors resulting from patternization
* Returns: 0 on success, -1 otherwise
*/
int write_pattern_send(pd_umem_pattern *umem_pat);

#endif
