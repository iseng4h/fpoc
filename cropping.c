#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>	
#include "fpoc.h"

#define THR 	0.99

void cropping(int *data, int height, int width, int *x1, int *x2, int *y1, int *y2)
{
        int *A , get;

	A=malloc(width*sizeof(int));
        //cropping
        get=0;
        for (x=0;x<width;x++) {
                A[x]=0;
                for (y=0;y<height;y++) {
                        A[x]=A[x]+ data[x+width*y];
                        //printf("%3.2ld ",(long int) *fftdata[x+width*y]);

                }
                //printf("A[%d]=%ld \n",x+1,A1[x]);

                if(get==0) {
                        if(A[x]<height*254*THR) {
                                *y1=x;
                                get=1;
                                //printf("%d %ld ",xa1,A1[x]);
                        }
                } else
                {
                        if(A[x]>height*254*THR) {
                                *y2=x;
                                get=0;
                                //printf("%d %ld ",x,A1[x]);
                        }
                }
        }

	
	A=realloc(A,height*sizeof(int));

        get=0;
        for (x=0;x<height;x++) {
                A[x]=0;
                for (y=0;y<width;y++) {
                        A[x]=A[x]+ data[y+width*x];
                        //printf("%d %3.2ld ",y+1, (long int)*fftdata[y+width*x]);

                }
                //printf("A[%d]=%ld \n",x+1,A2[x]);

                if(get==0) {
                        if(A[x]<width*254*THR) {
                                *x1=x;
                                get=1;
                                //printf("%d %ld ",ya1,A2[x]);
                        }
                } else
                {
                        if(A[x]>width*254*THR) {
                                *x2=x;
                                get=0;
                                //printf("%d %ld",ya2,A1[x]);
                        }
                }
        }

//	free(A);
        printf("Cropping Xmin=%d Xmax=%d Ymin=%d Ymax=%d.\n",*x1,*x2, *y1, *y2);
}

