#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "fftw3.h"
#include "fpoc.h"
#include "fftshift.h"


void pocfil(double *data1,  int N, int M, int a, int b, double result[])
{
	double *Real, *Imag, *hreal, *himag;
	int x1,x2,y1,y2;

	printf("POC-fil ");

	Real=malloc(N*M*sizeof(double));
	Imag=malloc(N*M*sizeof(double));
	hreal=malloc(N*M*sizeof(double));
	himag=malloc(N*M*sizeof(double));


	fdata1=fftw_malloc(N*M*sizeof(fftw_complex));

	for(x=0;x<N;x++) {
		for(y=0;y<M;y++) {
			fdata1[y+M*x][0]=data1[y+M*x];
		}
	}
	
	plan1=fftw_plan_dft_2d(N,M,fdata1,fdata1,FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_execute(plan1);

	for(x=0;x<N;x++) {
		for(y=0;y<M;y++) {
			Real[y+M*x]=fdata1[y+M*x][0];
			Imag[y+M*x]=fdata1[y+M*x][1];
		}
	}
	
	fftshift(0,Real,hreal,N,M);
	fftshift(0,Imag,himag,N,M);

	x1=(int)round((double)N/2)-a;
	x2=(int)round((double)N/2)+a;
	y1=(int)round((double)M/2)-b;
	y2=(int)round((double)M/2)+b;

	//*Xfil=x2-x1;
	//*Yfil=y2-y1;

	Real=realloc(Real,(x2-x1)*(y2-y1)*sizeof(double));
	Imag=realloc(Imag,(x2-x1)*(y2-y1)*sizeof(double));

	for(x=0;x<N;x++) {
		for(y=0;y<M;y++) {
			if( (x>x1) && (x<x2) && (y>y1) && (y<y2) ) {
				Real[(y-y1)+(y2-y1)*(x-x1)]=hreal[y+M*x];
				Imag[(y-y1)+(y2-y1)*(x-x1)]=himag[y+M*x];
			}
		}
	}
	
	hreal=realloc(hreal,(x2-x1)*(y2-y1)*sizeof(double));
	himag=realloc(himag,(x2-x1)*(y2-y1)*sizeof(double));

	fftshift(1,Real,hreal,(x2-x1),(y2-y1));
	fftshift(1,Imag,himag,(x2-x1),(y2-y1));

	for(x=0;x<(x2-x1);x++) {
		for(y=0;y<(y2-y1);y++) {
			fdata1[y+(y2-y1)*x][0]=Real[y+(y2-y1)*x];
			fdata1[y+(y2-y1)*x][1]=Imag[y+(y2-y1)*x];
		}
	}

	plan1=fftw_plan_dft_2d((x2-x1),(y2-y1),fdata1,fdata1,FFTW_BACKWARD, FFTW_ESTIMATE);
	fftw_execute(plan1);

	for(x=0;x<(x2-x1);x++) {
		for (y=0;y<(y2-y1);y++) {
			result[y+(y2-y1)*x]=fdata1[y+(y2-y1)*x][0];
		}
	}
	
//	fftw_free(fdata1);
//	fftw_destroy_plan(p1);

//	free(Real);free(Imag);free(hreal);free(himag);
	printf(".\n");
}

