#include <stdio.h>
#include <stdlib.h>

#define N 4096
#define M 4096
#define K 4096

int main(){
 short **matA, **matB, **matC;
 int i,j,k;

matA = (short**)malloc(sizeof(short*)*N);
matB = (short**)malloc(sizeof(short*)*N);
matC = (short**)malloc(sizeof(short*)*N);
if(matA == NULL || matB == NULL || matC == NULL)
	return -1;

for(i=0; i < N; i++){
   matA[i] = (short*)malloc(sizeof(short)*N);
   matB[i] = (short*)malloc(sizeof(short)*N);
   matC[i] = (short*)malloc(sizeof(short)*N);

   if(matA[i] == NULL || matB[i] == NULL || matC[i] == NULL)
      return -1;
}



printf("Computing...\n");
 
for(i=0;i<M;i++){
    for(j=0;j<K;j++){
        matC[i][j]=0;
        for(k=0;k<N;k++){
            matC[i][j]+=matA[i][k]*matB[j][k];
        }
    }
}

  return 0;
}
