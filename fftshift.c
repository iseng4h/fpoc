#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>	
#include <magick/api.h>
#include "fpoc.h"


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

