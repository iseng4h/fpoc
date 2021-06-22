#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>	

#include <magick/api.h>
#include "tiffio.h"
#include "mpi.h"
#include "fftw3.h"

#include "fpoc.h"
#include "align2.h"
#include "bandsize.h"
#include "cropping.h"
#include "fftshift.h"
#include "findmax.h"
#include "imrotate2.h"
#include "itoa.h"
#include "normalize.h"
#include "pocfil.h"
#include "poc.h"
#include "score.h"
#include "tiffprogram.h"

#define SDT 		12
#define SDTSTART 	-5

int main(int argc,char **argv)
{
	TIFF *tif1, *tif2;
	double  s[SDT+1], max;
	int aaa, lp1, lp2, xa1, xa2, ya1, ya2, N1, M1, N2, M2, sudut, angle[SDT+1];
	int sudutawal, sudutakhir,  jumlah;
	char filename[20], number[100];

	//Initialize MPI
	MPI_Status stat;
	MPI_Datatype indextype;
	
	int blklength[2], displace[2], tag=1,mpisend, mpirecv;
	double  genrecv[SDT+1], gensend[SDT+1];

	printf("Init MPI\n");
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&Me);
	MPI_Comm_size(MPI_COMM_WORLD,&NNodes);
	
	double scrsend,scrrecv,scrlast[NNodes];
	
	jumlah=SDT%NNodes;
	if(jumlah==0) {
		jumlah=SDT/NNodes;
	} else {
		printf("angle not for %d proc",NNodes);
		exit(0);
		
	}	

	blklength[0]=jumlah;
        displace[0]=0;
	MPI_Type_indexed(1,blklength,displace, MPI_DOUBLE, &indextype);
	MPI_Type_commit(&indextype);

	InitializeMagick(*argv);

//	Me=0;
//	jumlah=SDT;
	
//	ff=malloc(100*100*sizeof(double));


	if(Me==0) {
		//tif1 = TIFFOpen(argv[1], "r");
		tif1 = TIFFOpen("image1.tif", "r");
		ImgMalloc(tif1,&data1,&N1, &M1);
		ReadImg(tif1, data1);
		TIFFClose(tif1);

		cropping(data1,N1,M1, &xa1, &xa2, &ya1, &ya2);	
		WriteImg(data1,N1,M1, "crop1.tif",xa1,xa2,ya1,ya2);
	
		//tif2 = TIFFOpen(argv[2], "r");
		tif2 = TIFFOpen("image2.tif", "r");
		ImgMalloc(tif2,&data2,&N2, &M2);
		ReadImg(tif2, data2);
		TIFFClose(tif2);

		cropping(data2,N2,M2, &xa1, &xa2, &ya1, &ya2);	
		WriteImg(data2,N2,M2, "crop2.tif",xa1,xa2,ya1,ya2);
	
		free(data1);
		free(data2);
	}
	
	sudutawal=SDTSTART+((jumlah)*Me);
	sudutakhir=SDTSTART+((jumlah)*(Me+1))-1;
	

	strcpy(filename,"rotate");
	itoa(Me,number);
	strcat(filename,number);
	strcat(filename,".tif");
	
	MPI_Barrier(MPI_COMM_WORLD);

	aaa=0;
	for (sudut=sudutawal;sudut<=sudutakhir;sudut++){
//	sudut=-6;
		angle[aaa]=sudut;		
		imrotate2("crop1.tif",filename,sudut);	

		tif1 = TIFFOpen(filename, "r");
		tif2 = TIFFOpen("crop2.tif", "r");
		
		normalize(tif1,tif2,&N,&M);


		ff=malloc(N*M*sizeof(double));

		TIFFClose(tif1);
		TIFFClose(tif2);

		lp1=(int)round((double)N/4);
        	lp2=(int)round((double)M/4);

		poc(data1,data2,N,M,lp1,lp2,ff);
		
		findmax(ff,1,N,M,&max);
		s[aaa]=max;

		printf("%d. sudut[%d]=%f .\n",aaa,sudut,s[aaa]);

		aaa++;
//		sleep(100);

	}

	free(data1);free(data2);
	free(ff);

	
	printf("Waiting for Parallel Processing...\n");

	MPI_Barrier(MPI_COMM_WORLD);


//	Parallel thinks
	for(i=0;i<NNodes;i++) {
		if(Me==i) {
			printf("Send score\n");
			MPI_Send(gensend,1,indextype,0,tag,MPI_COMM_WORLD);
		}
		if(Me==0) {
			MPI_Recv(genrecv,1,indextype,i,tag,MPI_COMM_WORLD,&stat);
			for(j=0;j<jumlah;j++) {
				s[j+(jumlah*i)]=genrecv[j];
				printf("recv %f from %d\n",genrecv[j],i);
			}	
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}		

	MPI_Type_free(&indextype);
	
	if(Me==0) {
		for(x=0;x<SDT;x++) {
			printf("%f ",s[x]);	
		}
	}
	printf("\n");
	
	for(i=0;i<NNodes;i++) {
		if(Me==0) {
			findmax(s,0,1,SDT,&max);	
			s[(int)max]=(double)0;
			gensend[i]=SDTSTART+max;
			mpisend=SDTSTART+max;
			printf("Sending %3.1f ", gensend[i]);
			MPI_Send(&mpisend,1,MPI_INT,i,tag,MPI_COMM_WORLD);
		}
		if (Me==i) {
			MPI_Recv(&mpirecv,1,MPI_INT,0,tag,MPI_COMM_WORLD,&stat);
			printf("Recv data = %d\n",mpirecv);
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}	

	
//	findmax(s,0,1,jumlah,&max);

	int ff1x,ff1y;
	double scr ;

	printf("Matching...\n");

		//	for(loop=0;loop<4;loop++) {
		

		printf("angle=%3.1f degree\n",SDTSTART+max);
		imrotate2("crop1.tif",filename,SDTSTART+max);

		tif1 = TIFFOpen(filename, "r");
		tif2 = TIFFOpen("crop2.tif", "r");

		normalize(tif1,tif2,&N,&M);

	
		align2(data1,data2,N,M,&Ax,&Ay);

/*	
		if(Me==0) {
		WriteImg(al1,Ax,Ay,"test1.tif",0,Ax,0,Ay);
		WriteImg(al2,Ax,Ay,"test2.tif",0,Ax,0,Ay);
		}
		
*/
		
		bandsize(al2,Ax,Ay, &p1, &p2);	

		ff=malloc(Ax * Ay * sizeof(double));
		poc(al1,al2,Ax,Ay,0,0,ff);

		ff1x=(int)((round((double)Ax/2)+p1)-(round((double)Ax/2)-p1));
		ff1y=(int)((round((double)Ay/2)+p2)-(round((double)Ay/2)-p2));
	        ff1=malloc(ff1x * ff1y * sizeof(double));
		pocfil(ff,Ax,Ay, p2, p1, ff1);

		score(ff1,ff1x,ff1y,&scr);
		printf("Last Score from node %d = %5.4f\n",Me,scr);
//	}

	
	MPI_Barrier(MPI_COMM_WORLD);

	for(i=0;i<NNodes;i++) {
		if(Me==i) {
			scrsend=scr;
			MPI_Send(&scrsend,1,MPI_DOUBLE,0,tag,MPI_COMM_WORLD);
			printf("Send scr %f",scrsend);
		}
		if(Me==0) {
			MPI_Recv(&scrrecv,1,MPI_DOUBLE,i,tag,MPI_COMM_WORLD,&stat);
			scrlast[i]=scrrecv;
			printf("Recv scr from %d = %f",i,scrrecv);
		}	
	}
	
	if(Me==0) {
		findmax(scrlast,1,1,NNodes,&max);
		printf("Last Score %5.4f\n",max);
	}

	MPI_Finalize();

	TIFFClose(tif1);
	TIFFClose(tif2);
		
	free(ff);
	free(ff1);
	free(al1);free(al2);
	free(data1);free(data2);
	
	fftw_free(fdata1);
	fftw_free(fdata2);
	fftw_free(temp);

	fftw_destroy_plan(plan1);

	return (0);
}
