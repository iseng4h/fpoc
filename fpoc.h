#include<fftw3.h>

#ifndef FPOC_H
#define FPOC_H

extern double round(double x);
extern double sleep(double x);

int i,j,x,y,N,M,Ax,Ay,p1,p2;
int Me, NNodes, *data1,*data2, *al1, *al2;

double *ff, *ff1;

fftw_plan plan1;
fftw_complex *fdata1, *fdata2, *temp;


#endif
