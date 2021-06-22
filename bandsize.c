#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>	
#include "fftshift.h"
#include "fpoc.h"

void bandsize(int *al2, int N, int M, int *s1, int *s2)
{
	double *R, *hasil, *n1, *n2, n1min=0, n2min=0, n1mean=0, n2mean=0;
	int limit1, limit2, s1min=0, s1max=0, s2min=0, s2max=0, getmin=0, getmax=0;


	printf("Bandsize ");
	fdata1 = fftw_malloc(N * M *sizeof(fftw_complex));

	R=malloc(N*M*sizeof(double));
	hasil=malloc(N*M*sizeof(double));

	n1=malloc(M*sizeof(double));
	n2=malloc(N*sizeof(double));



	for(x=0;x<N;x++) {
		for(y=0;y<M;y++) {
			fdata1[y+M*x][0]=al2[y+M*x];
			fdata1[y+M*x][1]=0;
		}
	}
	
	plan1=fftw_plan_dft_2d(N, M, fdata1, fdata1, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(plan1);

	//abs
	for(x=0;x<N;x++) {
		for(y=0;y<M;y++) {
			R[y+M*x]=sqrt(pow(fdata1[y+M*x][0],2)+pow(fdata1[y+M*x][1],2));
		}
	}

	fftshift(0,R,hasil,N,M);
/*
	for(x=0;x<N;x++) {
		for (y=0;y<M;y++) {
			printf("%3.2d ",signal[y+M*x]);
		} 
		printf("\n");
	}	
*/
	for(x=0;x<N;x++) {
		n1[x]=0;
		for(y=0;y<M;y++) {
			n1[x]=n1[x]+hasil[y+M*x];
		}
//		printf("%3.2e \n",n1[x]);
		if (x==0) {
			n1min=n1[0];
		} else {	
			if(n1[x]<n1min) {
				n1min=n1[x];
			} 
		}
	}
	
	for(x=0;x<M;x++) {
		n2[x]=0;
		for(y=0;y<N;y++) {
			n2[x]=n2[x]+hasil[y*M+x];				
		}
		if (x==0) {
			n2min=n2[0];
		} else {
			if(n2[x]<n2min) {
				n2min=n2[x];
			}
		}
	}		

	limit1=(int)round((double)N/5);
	limit2=(int)round((double)M/5);
//	printf("%e %d\n",n1min,limit);	

	for(x=0;x<N;x++) {
		if ((x<=limit1) || (x>=N-limit1))  {
			n1[x]=n1min;
		}
		n1mean=n1mean+n1[x]; 
	}
	n1mean=n1mean/N;

	for(x=0;x<M;x++) {
		if((x<=limit2)||(x>=M-limit2)) {
			n2[x]=n2min;
		}
		n2mean=n2mean+n2[x];
	}	
	n2mean=n2mean/M;

	for(x=0;x<N;x++) {
		if(n1[x]>n1mean*0.9) {
			n1[x]=1;
		} else n1[x]=0;
	}
	for(x=0;x<N;x++){
		if(getmin==0) {
			if(n1[x]>0) {
				s1min=x;
				getmin=1;
			}
		}
		if (getmax==0) {
			if(n1[N-x-1]>0) {
				s1max=N-x-1;
				getmax=1;
			}
		}
	}
	getmin=0;getmax=0;
	for(x=0;x<M;x++) {
		if(n2[x]>n2mean*0.9) {
			n2[x]=1;
		} else n2[x]=0;
	}
	for(x=0;x<M;x++) {	
		if(getmin==0) {
			if(n2[x]>0) {
				s2min=x;
				getmin=1;
			}
		}
		if(getmax==0) {
			if(n2[M-x-1]>0) {
				s2max=M-x;
				getmax=1;
			}
		}
	}

//	printf("%d %d %d %d ",s1min,s1max,s2min,s2max);
	*s2=(int)round((double)(s1max-s1min)/2);
	*s1=(int)round((double)(s2max-s2min)/2);


//	for(x=0;x<M;x++) {
//		for (y=0;y<M;y++) {
//			printf("%d %d ",x,(int)n2[x]);
//		}
//	}	
	
	
	
//	free(R);free(hasil); 
//	free(n1);free(n2);
	

	printf(" p1=%d, p2=%d.\n",*s1,*s2);
}

