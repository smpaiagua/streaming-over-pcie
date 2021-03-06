/*******************************************************************
 * This is a test program for the C interface of the 
 * pciDriver library adaptaded to the VC707 Dev Board.
 * 
 * $Revision: 1.2 $
 * $Date: 2013-02-25 $
 * 
 *******************************************************************/

/*******************************************************************
 * Change History:
 * 
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2006-10-16 16:56:56  marcus
 * Initial version. Tests the C interface of the library.
 *
 *******************************************************************/

#include "/root/Documents/spaiagua/Swinger/pcie_dev_driver/pciDriver/include/lib/pciDriver.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_KBUF (8*1024*1024)
#define MAX_UBUF (64*1024*1024)

#define PCORE_BASE 0
#define PCIE_BASE (4096/4)
#define ACCEL_BASE0 (8192/4)
#define ACCEL_BASE1 (12288/4)

void testDevice( int i );
void PCIconfig(pd_device_t *dev);
void testPCImmap(pd_device_t *dev, int nbar, int opt);
void testKernelMemory(pd_device_t *dev);
void testUserMemory(pd_device_t *dev);

int main() 
{
	int i;
	pd_device_t dev;
	
	
	printf("VC707 PCIE Test on fpga0\n");
	
	// Opening device fpga0
	if(pd_open(0,&dev) != 0){
	    printf("Failed to open file \n");
	    return -1;
	}
	
	printf("Device file opened successfuly\n");
	
	PCIconfig(&dev);
	
	testPCImmap(&dev,1,2);
	
	
	pd_close( &dev );
	printf("Device file closed successfuly\n");


	return 0;
}

void testDevice( int i )
{
	pd_device_t dev;
	int ret;
	
	printf("Trying device %d ...", i);
	ret = pd_open( i, &dev );

	if (ret != 0) {
		printf("failed\n");
		return;
	}

	printf("ok\n");	

	PCIconfig(&dev);
	testPCImmap(&dev,1,0);
	testKernelMemory(&dev);
	testUserMemory(&dev);
	
	pd_close( &dev );
}

void PCIconfig(pd_device_t *dev)
{
	int i,j,ret;
	
	printf(" Testing PCI config ... \n");
	
	/*
	printf("  Reading PCI config area in byte mode ... \n");
	for(i=0;i<32;i++) {
		printf("   %03d: ", i*8 );
		for(j=0;j<8;j++) {
			ret = pd_readConfigByte( dev, (i*8)+j );
			printf("%02x ",ret);
		}
		printf("\n");
	}*/

	printf("  Reading PCI config area in word mode ... \n");
	for(i=0;i<32;i++) {
		printf("   %03d: ", i*8 );
		for(j=0;j<8;j+=2) {
			ret = pd_readConfigWord( dev, (i*8)+j );
			printf("%04x ",ret);
		}
		printf("\n");
	}
}

void testPCImmap(pd_device_t *dev, int nbar, int opt)
{
	int i,j,ret;
	int addr;
	void *bar;
	unsigned int size;
	unsigned int test_v[256];
	int res;
	
	for(i=0;i<nbar;i++) {
		size = pd_getBARsize( dev,i );
		printf( "BAR %d size: %d\n",i, size );
	  
		printf("Mapping BAR %d ...",i);
		bar = pd_mapBAR( dev, i );
		if (bar == NULL) {
			printf("failed\n");
			break;
		}
		printf("BAR %d mapped\n", i);
		
		if(opt == 0){
		
		    printf("accessing BAR %d: \n", i);
		    
		    printf("PCORE space: \n\n");
		    for(addr= PCORE_BASE;addr < PCORE_BASE + (4096/4); addr +=1){
			//printf("%02x: ", addr);
			printf("%08x ", ((unsigned int*)bar)[addr]);
		    }
		    
		    getchar();
		    
		    printf("PCIE AXI space: \n\n");
		    for(addr=PCIE_BASE;addr < PCIE_BASE + (4096/4); addr +=1){
			printf("%08x ", ((unsigned int*)bar)[addr]);
		    }
		    
		    getchar();
		    
		     printf("ACCEL0 AXI space: \n\n");
		    for(addr=PCIE_BASE;addr < ACCEL_BASE0 + (4096/4); addr +=1){
			printf("%08x ", ((unsigned int*)bar)[addr]);
		    }
		    
		     getchar();
		    
		     printf("ACCEL1 AXI space: \n\n");
		    for(addr=PCIE_BASE;addr < ACCEL_BASE1 + (4096/4); addr +=1){
			printf("%08x ", ((unsigned int*)bar)[addr]);
		    }
		    
		}
		else if(opt == 1){
		
		    printf("Running write/read test\n");
		  
		    ((unsigned int*)bar)[PCORE_BASE+1] = 0X5;
		    printf("%08x ", ((unsigned int*)bar)[PCORE_BASE+1]);
		    ((unsigned int*)bar)[PCORE_BASE+2] = 0X10;
		    printf("%08x ", ((unsigned int*)bar)[PCORE_BASE+2]);
		    ((unsigned int*)bar)[PCORE_BASE+3] = 0X10;
		    printf("%08x ", ((unsigned int*)bar)[PCORE_BASE+3]);
		}
		else if(opt == 2){
		    // Generate test vector
		    for(j=0; j< 256; j++){
			test_v[j] = j;
		    }
		    
		    // Write test vector to ACCELERATOR
		    for(j=0; j < 256; j++){
		      ((unsigned int*)bar)[ACCEL_BASE0 + j] = test_v[j];
		    }
		    
		    
		    // Enable start of computation
		      ((unsigned int*)bar)[ACCEL_BASE1] = 0xffffffff; // Write something to register 0 in MEM space 1
		      
		    printf("Setting up interrupts\n");
		    if (pd_waitForInterrupt(dev, 0 ) == -1){
			printf("Wait for interrupt failed \n");
			continue;
		    }
		    
		    printf("Interrupt received :D \n");
		      

		      
		    // Hopefully read result
		    do{
		    res = ((unsigned int*)bar)[ACCEL_BASE1 + 1];
		    }
		    while(res == 0);
		    printf("Result of vector sum: %08x\n\n",res);
		  
		}
		
		ret=pd_unmapBAR( dev,i,bar );
		 if (ret < 0) {
			    printf("failed\n");
			    continue;
		 }
		
		printf("Unmapped BAR %d \n", i);
	}
}

void testKernelMemory(pd_device_t *dev)
{
	pd_kmem_t km;
	void *buf;
	unsigned int size,ret;
	
	for(size=1024;size<=MAX_KBUF;size*=2) {
		printf("%d: ", size );
		
		buf = pd_allocKernelMemory( dev, size, &km );
		if (buf == NULL) {
			printf("failed\n");
			break;
		}
		printf("created ( %08x - %08x ) ", km.pa, km.size ); 

		ret = pd_freeKernelMemory( &km );
		if (ret < 0) {
			printf("failed\n");
			break;
		}
		printf("deleted\n" );
	}
}

void testUserMemory(pd_device_t *dev)
{
	pd_umem_t um;
	void *mem;
	unsigned int size,i,ret;
	
	for(size=1024;size<=MAX_UBUF;size*=2) {
		printf("%d: ", size );

		posix_memalign( (void**)&mem, 16, size );
		if (mem == NULL) {
			printf("failed\n");
			return;
		}

		ret = pd_mapUserMemory( dev, mem, size, &um );
		if (ret < 0) {
			printf("failed\n");
			break;
		}
		printf("mapped ( %d entries ) ", um.nents );

		ret = pd_unmapUserMemory( &um );
		if (ret < 0) {
			printf("failed\n");
			break;
		}
		printf("unmapped\n" );

		free( mem );
	}
	
	// SG example
	size = 1*1024*1024;	// 1MB
	posix_memalign( (void**)&mem, 16, size );

	printf("  Allocating %d bytes\n", size);

	ret = pd_mapUserMemory( dev, mem, size, &um );
	if (ret < 0) {
		printf("failed\n");
		return;
	}
		
	printf("   SG entries: %d\n", um.nents );
		
	for(i=0;i<um.nents;i++) {
		printf("%d: %08x - %08x\n", i, um.sg[i].addr, um.sg[i].size);
	}

	ret = pd_unmapUserMemory( &um );
	if (ret < 0) {
		printf("failed\n");
		return;
	}

	free( mem );
}

