#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>	
#include "fpoc.h"


void score(double *data, int N, int M , double *s)
{
	double max1=0, max2=0;

	for(x=0;x<N;x++) {
		for (y=0;y<M;y++) {
			if (data[y+M*x]<0) data[y+M*x]=data[y+M*x]*-1;
//			printf("%e \n",data[y+M*x]);
			if ((data[y+M*x]>max1)||(data[y+M*x]>max2)) {
				if(max1>max2) {
					max2=max1;
				}
				max1=data[y+M*x];
			}
		}
	}
	*s=max1+max2;
	
	printf("Score %3.2e + %3.2e=%3.2e .\n",max1,max2,*s);
}

