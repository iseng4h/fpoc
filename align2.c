#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>	
#include <magick/api.h>
#include "fftw3.h"
#include "tiffio.h"
#include "poc.h"
#include "fpoc.h"
#include "fftshift.h"

void align2(int *data1, int *data2, int N, int M, int *Ax, int *Ay)
{
	int *aa, *bb, xx, yy, N1, N2, xmax=0, ymax=0, regx, regy, ax, ay, bx, by, awalax, awalay, awalbx, awalby;
	int xa1=0,xa2=0,xb1=0,xb2=0,ya1=0,ya2=0,yb1=0,yb2=0;
	int A1[2*M], A2[2*N], B1[2*M], B2[2*N];
	double *R, *hasil, max=0;

	 
	N1=(int)round((double) N/2);
	N2=(int)round((double) M/2);

	xx=N1+N+N1;
	yy=N2+M+N2;

	printf("Align2 N1=%3.0d N2=%3.0d xx=%3.0d yy=%3.0d ",N1,N2,xx,yy);
	
	aa = malloc(xx * yy *sizeof(int));
	bb = malloc(xx * yy *sizeof(int));
	R = malloc(xx * yy *sizeof(double));
	hasil = malloc(xx * yy *sizeof(double));


	for(x=0;x<xx;x++) {
		for (y=0;y<yy;y++) {
			aa[y+yy*x]=254;
			bb[y+yy*x]=254;
			if ((y>N2) && (y<N2+M) && (x>N1) && (x<N1+N)) {
				aa[y+yy*x]=data1[(y-N2)+M*(x-N1)];
				bb[y+yy*x]=data2[(y-N2)+M*(x-N1)];
			}
		}
	}


	//REGISTRATION_INT
	printf("\nRegistration_int ");
	poc(aa,bb,xx,yy,(int)round((double)xx/4),(int)round((double)yy/4),R);
	

	//fftshift
	fftshift(0,R,hasil,xx,yy);	

	
	for(x=0;x<xx;x++) {
		for(y=0;y<yy;y++) {
			//if (hasil[(y+yy*x)]<0) hasil[(y+yy*x)]=hasil[y+yy*x]*-1;
			hasil[y+yy*x]=sqrt(pow(hasil[y+yy*x],2)+pow(0,2));
			
			if (hasil[y+yy*x]>max) {
				max=hasil[y+yy*x];
				xmax=x;
				ymax=y;
			} else {
				max=max;
				xmax=xmax;
				ymax=ymax;
			}  
		}
	}

	regx= xmax - ( xx/2 +1) +1;
	regy= ymax - ( yy/2 +1) +1;
	printf(" peak=%f xmax=%d ymax=%d,  regx=%d regy=%d \n",(float)max,xmax,ymax,regx,regy);

	//End of REGISTRATION_INT 

	int k=1;
	
		if (regx<0) {
			regx=abs(regx);
			awalax=regx;
			awalbx=0;
		} else { 
			awalax=0;
			awalbx=regx;
		}
		if (regy<0) {
			regy=abs(regy);
			awalay=regy;
			awalby=0;
		} else {
			awalay=0;
			awalby=regy;
		}

		ax=regx+N;ay=regy+M;
		bx=regx+N;by=regy+M;

		printf("Size X=%d Y=%d\n",ax,by);		


		aa=realloc(aa,ax*ay*sizeof(int));
		bb=realloc(bb,bx*by*sizeof(int));

		for(x=0;x<ax;x++) {
                        for (y=0;y<ay;y++) {
                                aa[y+ay*x]=254;
                                bb[y+ay*x]=254;
                        }
                }
	
		printf("awalax=%d awalay=%d awalbx=%d awalby=%d\n",awalax,awalay, awalbx,awalby);
                for(x=0;x<N;x++) {
                        for(y=0;y<M;y++) {
                              	aa[(y+awalay)+ay*(x+awalax)]=data1[y+M*x];
                              	bb[(y+awalby)+by*(x+awalbx)]=data2[y+M*x];
                        }
                }

/*	
                A1=malloc(ay*sizeof(int));
                A2=malloc(ax*sizeof(int));
                B1=malloc(by*sizeof(int));
                B2=malloc(bx*sizeof(int));
*/

                int getax, getbx, getay, getby;

                getax=0; getbx=0;
                for(x=0;x<ax;x++) {
                        A1[x]=0;
                        B1[x]=0;
                        for(y=0;y<ay;y++) {
                                A1[x]=A1[x]+aa[y+ay*x];
                                B1[x]=B1[x]+bb[y+ay*x];
//                              printf("[%d,%d]=%3.0d ",x,y,A[(y+ay*x)]);
                        }
//                      printf("A1[%d] %5ld ",x,A1[x]);


	              	if(getax==0) {
                                if(A1[x]<ay*254*k) {
                                        xa1=x;
                                     getax=1;
//					printf("min < %d",ay*254*k);
                                }
                        } else {
                                if(A1[x]>=ay*254*k) {
                                        xa2=x;
                                        getax=0;
//					printf("max > %d",ay*254*k);
                                }
                        }

                        if(getbx==0) {
                                if(B1[x]<by*254*k) {
                                        xb1=x;
                                        getbx=1;
                                }
                        } else {
                                if(B1[x]>=by*254*k) {
                                        xb2=x;
                                        getbx=0;
                                }
                        }
                }

                getay=0; getby=0;
                for(x=0;x<ay;x++) {
                        A2[x]=0;
                        B2[x]=0;
                        for(y=0;y<ax;y++) {
                                A2[x]=A2[x]+aa[y+ax*x];
                                B2[x]=B2[x]+bb[y+ax*x];
//                              printf("%d ",A[(y+ax*x)]);

                        }
//                      printf("= A2[%d] %d \n ",x,A2[x]);
//                      printf("= A1[%d] %5d \n",x,A2[x]);

                      	if(getay==0) {
                                if(A2[x]<ax*254*k) {
                                        ya1=x;
                                        getay=1;
                                }
                        } else {
                                if(A2[x]>=ax*254*k) {
                                        ya2=x;
                                        getay=0;
                                }

                        }
                        if(getby==0) {
                                if(B2[x]<bx*254*k) {
                                        yb1=x;
                                        getby=1;
                                }
                        } else {
                                if(B2[x]>=bx*254*k) {
                                        yb2=x;
                                        getby=0;
                                }

                        }
                }


              	printf("xa1=%d xa2=%d ya1=%d ya2=%d\n",xa1,xa2,ya1,ya2);
              	printf("xb1=%d xb2=%d yb1=%d yb2=%d\n",xb1,xb2,yb1,yb2);
		
		int tempx1, tempx2, tempy1, tempy2;

		if (xa1>xb1) {
			tempx1=xa1;
		} else {
			tempx1=xb1;
		}
		
		if (xa2>xb2) {
			tempx2=xb2;
		} else {
			tempx2=xa2;
		}

		if (ya1>yb1) {
			tempy1=ya1;
		} else {
			tempy1=yb1;
		}

		if (ya2>yb2) {
			tempy2=yb2;
		} else {
			tempy2=ya2;
		}
		
	
//		*x1align=tempx1;
//		*x2align=tempx2;
//		*y1align=tempy1;
//		*y2align=tempy2;

		*Ax=tempx2-tempx1;
		*Ay=tempy2-tempy1;

		printf("x1=%d x2=%d y1=%d y2=%d, New size X=%d Y=%d\n",tempx1,tempx2,tempy1,tempy2,*Ax,*Ay);

		al1 = malloc((tempx2-tempx1)*(tempy2-tempy1)*sizeof(int));
		al2 = malloc((tempx2-tempx1)*(tempy2-tempy1)*sizeof(int));
		
		for(x=0;x<ax;x++) {
			for (y=0;y<ay;y++) {
				if ((x>tempx1) && (x<tempx2) && (y>tempy1) && (y<tempy2)) {
					al1[(y-tempy1)+(tempy2-tempy1)*(x-tempx1)]=aa[y+ay*x];
					al2[(y-tempy1)+(tempy2-tempy1)*(x-tempx1)]=bb[y+ay*x];
				}
			}
		}



//	free(aa); free(bb);
//	free(hasil);
//	free(R);	

//	free(A1);free(A2);free(B1);free(B2);

	printf(". \n");
}

