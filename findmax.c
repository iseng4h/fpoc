#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>	
#include "fpoc.h"

void findmax(double *input, int rest, int N, int M, double *max)
{

	double temp, tempmax=0, rank=0;

	if(rest==1) {
		for(x=0;x<N;x++) {
			for (y=0;y<M;y++) {
				//if(input[y+M*x]<0) input[y+M*x]=input[y+M*x]*-1;
				input[y+M*x]=sqrt(pow(input[y+M*x],2)+pow(0,2));
				temp=input[y+M*x];
				if(temp>tempmax)
					tempmax=temp;
				else
					tempmax=tempmax;
			}
		}
	} else {
		for(x=0;x<N;x++) {
			for (y=0;y<M;y++) {
				//if(input[y+M*x]<0) input[y+M*x]=input[y+M*x]*-1;
				input[y+M*x]=sqrt(pow(input[y+M*x],2)+pow(0,2));
				temp=input[y+M*x];
				if(temp>tempmax) {
					tempmax=temp;
					rank=(double)y;
				} else {
					tempmax=tempmax;
				}	

				//printf("input[%d]=%f ",y,input[y+M*x]);
			}
		}
		tempmax=rank;
	}	
	*max=tempmax;
}

