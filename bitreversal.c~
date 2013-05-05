#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int *genLUT(int size)
{
  int *LUT;
  short *shortLUT;
  int entries,i,bit;
  
   entries = (int)pow(2,size);
   LUT = (int*)malloc(sizeof(int)*entries);
   if(LUT == NULL)
     return NULL;
   
   for(i=0;i< entries; i++)
   {
      LUT[i] = 0;
      for(bit=0; bit < size; bit++)
      {
	LUT[i] |= (i & (1<<bit)) << (size - 2*bit - 1);
	LUT[i] |= (i & (1<<(size - bit - 1))) >> (size - 2*bit - 1);
      }
   }
    
  return LUT;
}


int main(int argc, char **argv)
{
 int nbits, i;
 int *lut;
 
 nbits = atoi(argv[1]);
 
 lut = genLUT(nbits);
 
 // print LUT
 for(i=0; i < (int)pow(2,nbits); i++)
 {
    printf("%08x ", lut[i]);
 }
  
  
  return 0;
}
