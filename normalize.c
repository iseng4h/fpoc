#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>	

#include "tiffio.h"
#include "fpoc.h"

void normalize(TIFF* tif1, TIFF* tif2, int *N, int *M)
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

