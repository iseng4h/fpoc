#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>	
#include <magick/api.h>
#include "fpoc.h"
#include "itoa.h"

void imrotate(char file1[20], char file2[20], double sudut)
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
}


void imrotate2(char file1[20], char file2[20], double sudut)
{
	char filename[20], number[100];
	
        strcpy(filename,"temp");
        itoa(Me,number);
        strcat(filename,number);
        strcat(filename,".pnm");
				
	
	printf("ImRotate2 %s for %d degree --> %s", file1, (int)sudut, file2);
	imrotate(file1,filename,sudut);
	imrotate(filename,file2,0);
	printf(".\n");
}
