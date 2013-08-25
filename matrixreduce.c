#include <stdio.h>
#include <stdlib.h>

#define N 32
#define NBLOCKS (128*128*128)
#define LINE (128*128)
#define C 4096

int main(){
 short ***matIn, **matC;
 int i,j,k;

matIn = (short***)malloc(sizeof(short**)*NBLOCKS);
matC = (short**)malloc(sizeof(short*)*C);
if(matIn == NULL || matC == NULL)
	return -1;

for(i=0; i < 4096; i++){
   matC[i] = (short*)malloc(sizeof(short)*C);

   if(matC[i] == NULL)
      return -1;
}

for(i=0; i < NBLOCKS; i++)
{
   matIn[i] = (short**)malloc(sizeof(short*)*N);
   if (matIn[i] == NULL)
       return -1;

   for(j=0; i < N; j++)
   {
      matIn[i][j] = (short*)malloc(sizeof(short)*N);

      if(matIn[i][j] == NULL)
	return -1;
   }
}


printf("Reducing...\n");
 
for(i=0; i < C/N; i++)
{
  for(j=0; j < C/N; j++)
  {
    matC[i][j] = 0;
    for(k=i*LINE; k < i*LINE + LINE; k++)
      matC[i][j] += matIn[k][i][j];
  }
}


  return 0;
}
