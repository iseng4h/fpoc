#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>	
#include <magick/api.h>
#include "tiffio.h"
#include "fftw3.h"

#define THR 	0.99

extern double round(double x);
extern double sleep(double x);

int	showdata = 1;			/* show data */
int 	i,x,y,N,M,Ax,Ay; 

void imrotate2(char file1[20], char file2[20], double sudut)
{
	ExceptionInfo
          exception;

	Image
          *image,
          *images,
          *resize_image,
          *thumbnails;

	ImageInfo
          *image_info;

      	/*
	 *         Initialize the image info structure and read an image.
	 *               */

	printf("ImRotate2 %s for %d degree", file1, (int)sudut);
	GetExceptionInfo(&exception);
	image_info=CloneImageInfo((ImageInfo *) NULL);
	(void) strcpy(image_info->filename,file1);
	images=ReadImage(image_info,&exception);
	if (exception.severity != UndefinedException)
		CatchException(&exception);
	if (images == (Image *) NULL)
		exit(1);
	/*
	 *         Turn the images into a thumbnail sequence.
	 *               */
	thumbnails=NewImageList();
	while ((image=RemoveFirstImageFromList(&images)) != (Image *) NULL)
	{
		resize_image=RotateImage(image,sudut,&exception);
	        if (resize_image == (Image *) NULL)
		        MagickError(exception.severity,exception.reason,exception.description);
	        (void) AppendImageToList(&thumbnails,resize_image);
	        DestroyImage(image);
	}
	/*
	 *         Write the image as MIFF and destroy it.
	 *               */
	(void) strcpy(thumbnails->filename,file2);
	WriteImage(image_info,thumbnails);
	DestroyImageList(thumbnails);
	DestroyImageInfo(image_info);
	DestroyExceptionInfo(&exception);
	DestroyMagick();
	printf(".\n");
}

void ReadImg(TIFF* tif, int *data)
{
        char *buf;
//	int *data;
        buf = _TIFFmalloc(TIFFStripSize(tif));
	uint32 height, width, row, rowsperstrip;

	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);

	for (row= 0; row < height; row+=rowsperstrip) {
                uint32 nrow = (row+rowsperstrip > height ? height-row : rowsperstrip);
                tstrip_t strip=TIFFComputeStrip(tif,row,0);
                //printf("size %dx%d, strip %d, row/strip %d nrow %d\n",height, width, strip,rowsperstrip,nrow);
                if (TIFFReadEncodedStrip(tif, strip, buf, nrow*width)<0) {
                        printf("Error");
                } else {
                        for (x=0;x<nrow;x++) {
                                for(y=0;y<width;y++){
                                        //if (showdata) printf("[%dx%d]=%d ",x+rowsperstrip*strip,y+width,(unsigned char) buf[y+width*x]);
                                        if (y<width-2) {
                                                if((int)buf[y+width*x]<0)
                                                        data[y+width*(x+rowsperstrip*strip)]=256+buf[y+width*x];
                                                else
                                                        data[y+width*(x+rowsperstrip*strip)]=(int)buf[y+width*x];
                                        } else
                                                data[y+width*(x+rowsperstrip*strip)]=254; //ngilangin pinggir kanan ben putih
                                        //if (showdata) printf("%3.2d ",(int)fftdata[y+width*(x+rowsperstrip*strip)][0]);
                                }
                                //putchar('\n');
                        }
                }
        }
        _TIFFfree(buf);

	printf("Read Image\n");
}

void ImgMalloc(TIFF *tif, int **data,int *Himg, int *Wimg)
{
	uint32 height, width, rowsperstrip;

	
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);

	*Himg=height;
	*Wimg=width;

	*data=malloc(height*width*sizeof(int));

}

void fftshift(int invers, double *in, double shift[], int xx, int yy)
{
	int nn,mm, checkx, checky;
	

//        in=malloc(xx*yy*sizeof(int));
//        shift=malloc(xx*yy*sizeof(double));

/*        for (x=0;x<xx;x++) {
                for (y=0;y<yy;y++) {
                        in[y+yy*x]=x*y;
                        printf("%d ",in[y+yy*x]);
                }
                printf("\n");
        }
*/
        if (invers) {
                nn=(int)floor((double)xx/2);
                mm=(int)floor((double)yy/2);
                checkx=xx%2;
                checky=yy%2;
                printf("ifftshift x=%d y=%d",xx,yy);
        } else {
                nn=(int)ceil((double)xx/2);
                mm=(int)ceil((double)yy/2);
                checkx=(xx%2)*-1;
                checky=(yy%2)*-1;
                printf("fftshift x=%d y=%d",xx,yy);
        }

//        printf("nn=%d mm=%d\n",nn,mm);

        //Bag 1 - 3
        for(x=0;x<nn;x++) {
                for (y=0;y<mm;y++) {
                        shift[(y+mm+checky)+yy*(x+nn+checkx)]=in[y+yy*x];
                }
        }
        //Bag 3 - 1
        for(x=nn;x<xx;x++) {
                for (y=mm;y<yy;y++) {
                        shift[(y-mm)+yy*(x-nn)]=in[y+yy*x];
                }
        }

        // Bag 2 - 4
        for (x=nn;x<xx;x++) {
                for (y=0;y<mm;y++) {
                        shift[(y+mm+checky)+yy*(x-nn)]=in[y+yy*x];
                }
        }

        // Bag 4 - 2
        for(x=0;x<nn;x++) {
                for(y=mm;y<yy;y++) {
                        shift[(y-mm)+yy*(x+nn+checkx)]=in[y+yy*x];
                }
        }
        //End fftshift

/*        for (x=0;x<xx;x++) {
                for (y=0;y<yy;y++) {
                        printf("%9.2e ",shift[y+yy*x]);
                }
                printf("\n");
        }
*/
//	free(shift);
	printf(".");
}

void WriteImg(int *dataimg,int N, int M,char file[20], int x1, int x2, int y1, int y2)
{
  // Define an image
  char *buffer;
  TIFF *image;
  int buf=0;
	
  buffer = malloc((x2-x1)*(y2-y1));

  for (x=x1;x<x2;x++) {
	for(y=y1;y<y2;y++) {
		buffer[buf]=(char)dataimg[y+M*x];
		buf++;
	}
  } 

  // Open the TIFF file
  if((image = TIFFOpen(file, "w")) == NULL){
    printf("Could not open TIFF file for writing\n");
    exit(42);
  }

  // We need to set some values for basic tags before we can add any data
  TIFFSetField(image, TIFFTAG_IMAGEWIDTH, y2-y1);
  TIFFSetField(image, TIFFTAG_IMAGELENGTH, x2-x1);
  //TIFFSetField(image, TIFFTAG_SUBFILETYPE, 0);
  TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, 8);
  TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, 1);
  TIFFSetField(image, TIFFTAG_ROWSPERSTRIP, x2-x1);

  TIFFSetField(image, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
  TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
  //TIFFSetField(image, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
  TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  //TIFFSetField(image, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

  //TIFFSetField(image, TIFFTAG_XRESOLUTION, 72.0);
  //TIFFSetField(image, TIFFTAG_YRESOLUTION, 72.0);
  //TIFFSetField(image, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

  // Write the information to the file
  TIFFWriteEncodedStrip(image, 0, buffer, (y2-y1)*(x2-x1));

  printf("Output file : %s.\n", file);

  free(buffer);	
  // Close the file
  TIFFClose(image);


}

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

void pocfil(double *data1,  int N, int M, int a, int b, double result[])
{
	fftw_complex *fdata1;
	fftw_plan p1;
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
	
	p1=fftw_plan_dft_2d(N,M,fdata1,fdata1,FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_execute(p1);

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

	p1=fftw_plan_dft_2d((x2-x1),(y2-y1),fdata1,fdata1,FFTW_BACKWARD, FFTW_ESTIMATE);
	fftw_execute(p1);

	for(x=0;x<(x2-x1);x++) {
		for (y=0;y<(y2-y1);y++) {
			result[y+(y2-y1)*x]=fdata1[y+(y2-y1)*x][0];
		}
	}
	
	fftw_free(fdata1);
	fftw_destroy_plan(p1);

	free(Real);free(Imag);free(hreal);free(himag);
	printf(".\n");
}

void poc(int *data1, int *data2, int N, int M, int lpx, int lpy, double result[])
{
	int x1,x2,y1,y2, nn, mm;
	double  *R, *hasil;
	fftw_plan p1, p2, ptemp;
	fftw_complex *fdata1, *fdata2, *temp;
	fftw_complex *frdata1, *frdata2, *rtemp;

	fdata1 = fftw_malloc(N * M *sizeof(fftw_complex));
	frdata1 = fftw_malloc(N * M *sizeof(fftw_complex));
	fdata2 = fftw_malloc(N * M *sizeof(fftw_complex));
	frdata2 = fftw_malloc(N * M *sizeof(fftw_complex));
	temp = fftw_malloc(N * M *sizeof(fftw_complex));
	rtemp = fftw_malloc(N * M *sizeof(fftw_complex));

	R=malloc(N*M*sizeof(double));
	hasil=malloc(N*M*sizeof(double));
	
	for(x=0;x<N;x++) {
		for (y=0;y<M;y++) {
			fdata1[y+M*x][0]=data1[y+M*x];
			fdata1[y+M*x][1]=0;
			fdata2[y+M*x][0]=data2[y+M*x];
			fdata2[y+M*x][1]=0;
		}
	}
	
	p1 = fftw_plan_dft_2d(N, M, fdata1, frdata1, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(p1);
        p2 = fftw_plan_dft_2d(N, M, fdata2, frdata2, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(p2);
	
	for(x=0;x<N;x++) {
                for (y=0;y<M;y++) {
                        temp[(y+M*x)][0]=cos(atan2(frdata1[(y+M*x)][1],frdata1[(y+M*x)][0])-atan2(frdata2[(y+M*x)][1],frdata2[(y+M*x)][0]));
                        temp[(y+M*x)][1]=sin(atan2(frdata1[(y+M*x)][1],frdata1[(y+M*x)][0])-atan2(frdata2[(y+M*x)][1],frdata2[(y+M*x)][0]));
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
        ptemp = fftw_plan_dft_2d(N,M, temp, rtemp, FFTW_BACKWARD, FFTW_ESTIMATE);
        fftw_execute(ptemp);

	//output
        for(x=0;x<N;x++) {
                for (y=0;y<M;y++) {
                        result[(y+M*x)]=rtemp[(y+M*x)][0]/(N*M);
			//printf("[%d,%d]=%9.2e %9.2ei ",x,y,out[y+M*x][0]);
			//printf("[%d,%d]=%9.2e %9.2ei ",x,y,temp[y+M*x][0],temp[y+M*x][1]);
                }
		//printf("\n");
        }
	
	free(R);
	free(hasil);

	fftw_free(temp);
	fftw_free(rtemp);
	fftw_free(fdata1);
	fftw_free(frdata1);
	fftw_free(fdata2);
	fftw_free(frdata2);

	fftw_destroy_plan(p1);
	fftw_destroy_plan(p2);
      	fftw_destroy_plan(ptemp);

	printf(".\n");
}


void normalize(TIFF* tif1, TIFF* tif2,int *data1, int *data2,int *N, int *M)
{
	char *buf1, *buf2;
	buf1 = _TIFFmalloc(TIFFStripSize(tif1));
	buf2 = _TIFFmalloc(TIFFStripSize(tif2));
	uint32 h1, w1, rs1;
	uint32 h2, w2, rs2;
	int N1, N2, c1, c2, x1,x2,y1,y2;
	int row, height, width, rowsperstrip;
		
	TIFFGetField(tif1, TIFFTAG_IMAGELENGTH, &h1);
	TIFFGetField(tif1, TIFFTAG_IMAGEWIDTH, &w1);
	TIFFGetField(tif1, TIFFTAG_ROWSPERSTRIP, &rs1);
	
	TIFFGetField(tif2, TIFFTAG_IMAGELENGTH, &h2);
	TIFFGetField(tif2, TIFFTAG_IMAGEWIDTH, &w2);
	TIFFGetField(tif2, TIFFTAG_ROWSPERSTRIP, &rs2);

	printf("Normalize H1=%d W1=%d R1=%d, H2=%d W2=%d R2=%d ",(int)h1,(int)w1,(int)rs1,(int)h2,(int)w2,(int)rs2);

	N1= h1;
	N2= w1;

	*N=N1;
	*M=N2;

	if (N1<h2) N1=h2;
	if (N2<w2) N2=w2;

	data1 = malloc(N1 * N2 *sizeof(int));	
	data2 = malloc(N1 * N2 *sizeof(int));	
	
	printf("--> N=%d M=%d ",N1,N2);

	for(x=0;x<N1;x++) {
		for(y=0;y<N2;y++) {
			data1[y+N2*x]=254;
			data2[y+N2*x]=254;
		}
	}

	c1=(N1/2);
	c2=(N2/2);

	//printf("c1=%d c2=%d\n",c1,c2);	

	x1=c1-(h1/2);x2=h1+(c1-(h1/2));
 	y1=c2-(w1/2);y2=w1+(c2-(w1/2));	

	//printf("x1=%d x2=%d y1=%d y2=%d\n",x1,x2,y1,y2);	
	
	height=h1;
	width=w1;
	rowsperstrip=rs1;

  	for (row= 0; row < height; row+=rowsperstrip) {
                uint32 nrow = (row+rowsperstrip > height ? height-row : rowsperstrip);
                tstrip_t strip=TIFFComputeStrip(tif1,row,0);
                //printf("size %dx%d, strip %d, row/strip %d nrow %d\n",height, width, strip,rowsperstrip,nrow);
                if (TIFFReadEncodedStrip(tif1, strip, buf1, nrow*width)<0) {
                        printf("Error");
                } else {
                        for (x=0;x<nrow;x++) {
                                for(y=0;y<width;y++){
                                        //if (showdata) printf("[%dx%d]=%d ",x+rowsperstrip*strip,y+width,(unsigned char) buf[y+width*x]);
                                                if((int)buf1[y+width*x]<0)
                                                        data1[(y+y1)+N2*((x+x1)+rowsperstrip*strip)]=256+(int)buf1[y+width*x];
                                                else
                                                        data1[(y+y1)+N2*((x+x1)+rowsperstrip*strip)]=(int)buf1[y+width*x];
                                        //if (showdata) printf("%3.2d ",(int)data[y+width*(x+rowsperstrip*strip)][0]);

                                                //if((int)buf1[y+width*x]==-1)
                                                //        data1[(y+y1)+N2*((x+x1)+rowsperstrip*strip)][0]=254;
                                }
                                //putchar('\n');
                        }
                }
        }

	x1=c1-(h2/2);x2=h2+(c1-(h2/2));
 	y1=c2-(w2/2);y2=w2+(c2-(w2/2));	

	//printf("x1=%d x2=%d y1=%d y2=%d\n",x1,x2,y1,y2);	

	height=h2;
	width=w2;
	rowsperstrip=rs2;

  	for (row= 0; row < height; row+=rowsperstrip) {
                uint32 nrow = (row+rowsperstrip > height ? height-row : rowsperstrip);
                tstrip_t strip=TIFFComputeStrip(tif2,row,0);
                //printf("size %dx%d, strip %d, row/strip %d nrow %d\n",height, width, strip,rowsperstrip,nrow);
                if (TIFFReadEncodedStrip(tif2, strip, buf2, nrow*width)<0) {
                        printf("Error");
                } else {
                        for (x=0;x<nrow;x++) {
                                for(y=0;y<width;y++){
                                        //if (showdata) printf("[%dx%d]=%d ",x+rowsperstrip*strip,y+width,(unsigned char) buf[y+width*x]);
                                                if((int)buf2[y+width*x]<0)
                                                        data2[(y+y1)+N2*((x+x1)+rowsperstrip*strip)]=256+(int)buf2[y+width*x];
                                                else
                                                        data2[(y+y1)+N2*((x+x1)+rowsperstrip*strip)]=(int)buf2[y+width*x];
                                        //if (showdata) printf("%3.2d ",(int)data2[y+*(x+rowsperstrip*strip)][0]);
                                                
						//if((int)buf2[y+width*x]==-1)
                                                //        data2[(y+y1)+N2*((x+x1)+rowsperstrip*strip)][0]=254;
                                }
                                //putchar('\n');
                        }
                }
        }

	_TIFFfree(buf1);
	_TIFFfree(buf2);

	printf(".\n");
}

void bandsize(int *signal, int N, int M, int *s1, int *s2)
{
	fftw_complex *bandtemp;
	fftw_plan fft2band;
	double *R, *hasil, *n1, *n2, n1min=0, n2min=0, n1mean=0, n2mean=0;
	int limit1, limit2, s1min=0, s1max=0, s2min=0, s2max=0, getmin=0, getmax=0;


	R=malloc(N*M*sizeof(double));
	hasil=malloc(N*M*sizeof(double));

	n1=malloc(M*sizeof(double));
	n2=malloc(N*sizeof(double));

	printf("Bandsize ");
	bandtemp = fftw_malloc(N*M*sizeof(fftw_complex));

	for(x=0;x<N;x++) {
		for(y=0;y<M;y++) {
			bandtemp[y+M*x][0]=signal[y+M*x];
			bandtemp[y+M*x][1]=0;
		}
	}
	
	fft2band=fftw_plan_dft_2d(N, M, bandtemp, bandtemp, FFTW_FORWARD, FFTW_ESTIMATE);
        fftw_execute(fft2band);

	//abs
	for(x=0;x<N;x++) {
		for(y=0;y<M;y++) {
			R[y+M*x]=sqrt(pow(bandtemp[y+M*x][0],2)+pow(bandtemp[y+M*x][1],2));
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
	
	
	fftw_free(bandtemp);
	fftw_destroy_plan(fft2band);
	
//	free(R);free(hasil);
//	free(n1);free(n2);

	printf(" p1=%d, p2=%d.\n",*s1,*s2);
}

void align2(int *data1, int *data2, int N, int M, int **al1, int **al2, int *Ax, int *Ay)
{
	int *aa, *bb, xx, yy, N1, N2, xmax=0, ymax=0, regx, regy, ax, ay, bx, by, awalax, awalay, awalbx, awalby;
	int xa1=0,xa2=0,xb1=0,xb2=0,ya1=0,ya2=0,yb1=0,yb2=0;
	int *A1, *A2, *B1, *B2;
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
			if (hasil[(y+yy*x)]<0) hasil[(y+yy*x)]=hasil[y+yy*x]*-1;
			
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
	printf("peak=%f xmax=%d ymax=%d regx=%d regy=%d \n",(float)max,xmax,ymax,regx,regy);

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

		printf("size ax=bx=%d ay=by=%d\n",ax,by);		


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

                A1=malloc(ay*sizeof(int));
                A2=malloc(ax*sizeof(int));
                B1=malloc(by*sizeof(int));
                B2=malloc(bx*sizeof(int));

	        int getax, getbx, getay, getby;
		
                getay=0; getby=0;
                for(x=0;x<ay;x++) {
                        A1[x]=0;
                        B1[x]=0;
                        for(y=0;y<ax;y++) {
                                A1[x]=A1[x]+aa[y*ay+x];
                                B1[x]=B1[x]+bb[y*by+x];

                        }
//                      printf("A1[%d] %d \n",x,A1[x]);
//                      printf("= B2[%d] %5ld \n",x,B2[x]);

                      	if(getay==0) {
                                if(A1[x]<ax*254*k) {
                                        ya1=x;
                                        getay=1;
                                }
                        } else {
                                if(A1[x]>=ax*254*k) {
                                        ya2=x-1;
                                        getay=0;
                                } else ya2=ay;

                        }
                        if(getby==0) {
                                if(B1[x]<bx*254*k) {
                                        yb1=x;
                                        getby=1;
                                }
                        } else {
                                if(B1[x]>=bx*254*k) {
                                        yb2=x-1;
                                        getby=0;
                                } else yb2=ay;

                        }
                }

                getax=0; getbx=0;
                for(x=0;x<ax;x++) {
                        A2[x]=0;
                        B2[x]=0;
                        for(y=0;y<ay;y++) {
                                A2[x]=A2[x]+aa[y+ay*x];
                                B2[x]=B2[x]+bb[y+ay*x];
//                              printf("[%d,%d]=%3.0d ",x,y,A[(y+ay*x)]);
                        }
//                      printf("A1[%d] %5ld ",x,A1[x]);
//                      printf("A1[%d] %5ld \n",x,A1[x]);

	              	if(getax==0) {
                                if(A2[x]<ay*254*k) {
                                        xa1=x;
                                     getax=1;
//					printf("min < %d",ay*254*k);
                                } 
                        } else {
                                if(A2[x]>=ay*254*k) {
                                        xa2=x-1;
                                        getax=0;
//					printf("max > %d",ay*254*k);
                                } else xa2=ax;
                        }

                        if(getbx==0) {
                                if(B2[x]<by*254*k) {
                                        xb1=x;
                                        getbx=1;
                                }
                        } else {
                                if(B2[x]>=by*254*k) {
                                        xb2=x-1;
                                        getbx=0;
                                } else xb2=ax;
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

		printf("x1=%d x2=%d y1=%d y2=%d, Size x=%d y=%d\n",tempx1,tempx2,tempy1,tempy2,*Ax,*Ay);

		*al1 = malloc((tempx2-tempx1)*(tempy2-tempy1)*sizeof(int));
		*al2 = malloc((tempx2-tempx1)*(tempy2-tempy1)*sizeof(int));

/*
		for(x=0;x<ax;x++) {
			for (y=0;y<ay;y++) {
				if ((x>tempx1) && (x<tempx2) && (y>tempy1) && (y<tempy2)) {
					al1[(y-tempy1)+(tempy2-tempy1)*(x-tempx1)]=aa[y+ay*x];
					al2[(y-tempy1)+(tempy2-tempy1)*(x-tempx1)]=bb[y+ay*x];
				}
			}
		}
*/
/*
	for(x=0;x<ax;x++) {
		for(y=0;y<ay;y++) {
			printf("%3.0d ",A[(y+ay*x)]);
		}
		printf("\n");
	}
*/

	free(aa); free(bb);
	free(R); free(hasil);

	free(A1);free(A2);free(B1);free(B2);

	printf(". \n");
}

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

	free(A);
        printf("Cropping Xmin=%d Xmax=%d Ymin=%d Ymax=%d.\n",*x1,*x2, *y1, *y2);
}

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
			}
		}
		tempmax=rank;
	}	
	*max=tempmax;
}

int main(int argc, char* argv[])
{
	TIFF *tif1, *tif2;
	int *al1, *al2, Ax, Ay, p1, p2, ff1x, ff1y; 
	double *ff, *ff1, s[101], s1[5] ,max, scr ;
	int *data1, *data2, loop, aaa,Nlp1,Nlp2, xa1, xa2, ya1, ya2, N1, M1, N2, M2, sudut, angle[101];
	
        InitializeMagick(*argv);
//	ff=malloc(sizeof(double));
	
	tif1 = TIFFOpen(argv[1], "r");
	ImgMalloc(tif1,&data1,&N, &M);
	ReadImg(tif1, data1);
	TIFFClose(tif1);

	cropping(data1,N,M, &xa1, &xa2, &ya1, &ya2);	
	WriteImg(data1,N,M, "crop1.tif",xa1,xa2,ya1,ya2);
	
	tif2 = TIFFOpen(argv[2], "r");
	ImgMalloc(tif2,&data2,&N, &M);
	ReadImg(tif2, data2);
	TIFFClose(tif2);

	cropping(data2,N,M, &xa1, &xa2, &ya1, &ya2);	
	WriteImg(data2,N,M, "crop2.tif",xa1,xa2,ya1,ya2);
	
	free(data1);
	free(data2);

	aaa=0;
	for (sudut=0;sudut<=5;sudut++){
//	sudut=0;
		angle[aaa]=sudut;		
		imrotate2("crop1.tif","rotate.tif",sudut);	

		tif1 = TIFFOpen("rotate.tif", "r");
		ImgMalloc(tif1,&data1,&N1, &M1);

		tif2 = TIFFOpen("crop2.tif", "r");
		ImgMalloc(tif2,&data2,&N2, &M2);
		
		normalize(tif1,tif2,data1,data2,&N,&M);

		TIFFClose(tif1);
		TIFFClose(tif2);

		Nlp1=(int)round((double) N/4);
        	Nlp2=(int)round((double) M/4);

		ff=malloc(N*M*sizeof(double));
		poc(data1,data2,N,M,Nlp1,Nlp2,ff);
		
		findmax(ff,1,N,M,&max);
		s[aaa]=max;

		printf("list[%d] sudut[%d]=%f\n",aaa,sudut,s[aaa]);
		
		free(data1);free(data2);
		free(ff);

		aaa++;
//		sleep(1);
	}
	
	for(loop=0;loop<4;loop++) {
		findmax(s,0,1,aaa,&max);
		s[angle[(int)max]]=0;
		imrotate2("crop1.tif","rotate2.tif",angle[(int)max]);

		tif1 = TIFFOpen("rotate2.tif", "r");
		ImgMalloc(tif1,&data1,&N1, &M1);

		tif2 = TIFFOpen("crop2.tif", "r");
		ImgMalloc(tif2,&data2,&N2, &M2);

		normalize(tif1,tif2,data1,data2,&N,&M);

		TIFFClose(tif1);
		TIFFClose(tif2);
		
		align2(data1,data2,N,M,&al1,&al2,&Ax,&Ay);
/*		
		bandsize(al2,Ax,Ay, &p1, &p2);	
																
		ff=malloc(Ax * Ay * sizeof(double));
		poc(al1,al2,Ax,Ay,0,0,ff);

		ff1x=(int)((round((double)Ax/2)+p1)-(round((double)Ax/2)-p1));
		ff1y=(int)((round((double)Ay/2)+p2)-(round((double)Ay/2)-p2));

		
	        ff1=malloc(ff1x * ff1y * sizeof(double));
		pocfil(ff,Ax,Ay, p2, p1,ff1);

		score(ff1,ff1x,ff1y,&scr);
		s1[loop]=scr;
*/
		free(ff);free(ff1);
		free(al1);free(al2);
	}
	
	return (0);
}
