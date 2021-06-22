#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>	
#include <magick/api.h>
#include "fftw3.h"
#include "fpoc.h"
#include "fftshift.h"

void  poc(int *data1, int *data2, int N, int M, int lpx, int lpy, double *Result)
{
	int x1,x2,y1,y2, nn, mm;
	double *R, *hasil;

	R=malloc(N*M*sizeof(double));
	hasil=malloc(N*M*sizeof(double));
	
	fdata1 = fftw_malloc(N * M *sizeof(fftw_complex));
	fdata2 = fftw_malloc(N * M *sizeof(fftw_complex));
	temp = fftw_malloc(N * M *sizeof(fftw_complex));

	
	for(x=0;x<N;x++) {
		for (y=0;y<M;y++) {
			fdata1[y+M*x][0]=data1[y+M*x];
			fdata1[y+M*x][1]=0;
			fdata2[y+M*x][0]=data2[y+M*x];
			fdata2[y+M*x][1]=0;
		}
	}
	
	plan1 = fftw_plan_dft_2d(N, M, fdata1, fdata1, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(plan1);
        plan1 = fftw_plan_dft_2d(N, M, fdata2, fdata2, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(plan1);
	
	for(x=0;x<N;x++) {
                for (y=0;y<M;y++) {
                        temp[(y+M*x)][0]=cos(atan2(fdata1[(y+M*x)][1],fdata1[(y+M*x)][0])-atan2(fdata2[(y+M*x)][1],fdata2[(y+M*x)][0]));
                        temp[(y+M*x)][1]=sin(atan2(fdata1[(y+M*x)][1],fdata1[(y+M*x)][0])-atan2(fdata2[(y+M*x)][1],fdata2[(y+M*x)][0]));
			//printf("%9.2e ",(double)atan2((double)fdata1[(y+M*x)][1],(double)fdata1[(y+M*x)][0])-(double)atan2(fdata2[(y+M*x)][1],(double)fdata2[(y+M*x)][0]));
                }
		//printf("\n");
        }
	
	//low-pas filter
	mm=(int)round((double)M/2);
	nn=(int)round((double)N/2);

	x1=nn-lpx;
	x2=nn+lpx;
	y1=mm-lpy;
	y2=mm+lpy;

	//printf("x1=%d x2=%d y1=%d y2=%d \n", x1, x2, y1, y2);

	if ((lpx!=0) || (lpx!=0)) {
		printf("POC-LP %d %d %d %d ", N, M, lpx, lpy);
		for(x=0;x<N;x++) {
			for (y=0;y<M;y++) {
				R[y+M*x]=0;
				if((y>=y1-1)&&(y<y2)&&(x>=x1-1)&&(x<x2)) {
					R[y+M*x]=1;
				}
			}
		}

		//ifftshift
		fftshift(1,R,hasil,N,M);

		//R.*
		for(x=0;x<N;x++) {
			for(y=0;y<M;y++) {
				temp[y+M*x][0]=temp[y+M*x][0]*hasil[y+M*x];
				temp[y+M*x][1]=temp[y+M*x][1]*hasil[y+M*x];
			}
		}
	} else {
		printf("POC %d %d ", N,M);
		for(x=0;x<N;x++) {
			for(y=0;y<M;y++) {
				temp[y+M*x][0]=temp[y+M*x][0]*1;
				temp[y+M*x][1]=temp[y+M*x][1]*1;
			}
		}
	}
        plan1 = fftw_plan_dft_2d(N,M, temp, temp, FFTW_BACKWARD, FFTW_ESTIMATE);
        fftw_execute(plan1);

	//output
        for(x=0;x<N;x++) {
                for (y=0;y<M;y++) {
                        Result[(y+M*x)]=temp[(y+M*x)][0]/(N*M);
			//printf("[%d,%d]=%9.2e %9.2ei ",x,y,out[y+M*x][0]);
			//printf("[%d,%d]=%9.2e %9.2ei ",x,y,temp[y+M*x][0],temp[y+M*x][1]);
                }
		//printf("\n");
        }
	

	free(R);
	free(hasil);

	fftw_free(temp);
	fftw_free(fdata1);
	fftw_free(fdata2);

	fftw_destroy_plan(plan1);

	printf(".\n");
}


