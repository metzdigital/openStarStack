#include <stdlib.h>
#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>
#include "pixelLib.h"
#include "starAlign.h"
#include "starStack.h"
#include "fileManage.h"

IplImage *RotateImage(IplImage *src, const IplImage *imgPrimary, float angle) {
	unsigned int width, height;
	CvPoint2D32f center;

	width = imgPrimary->width;
	height = imgPrimary->height;

	IplImage *rotatedImage = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, imgPrimary->nChannels);

	center.x = (float)(src->width)/2.0;
	center.y = (float)(src->height)/2.0;
	
	CvMat *mapMatrix = cvCreateMat( 2, 3, CV_32FC1 );

	cv2DRotationMatrix(center, angle, 1.0, mapMatrix);
	cvWarpAffine(src, rotatedImage, mapMatrix, CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS, cvScalarAll(0));

	cvReleaseImage(&src);
	cvReleaseMat(&mapMatrix);

	return rotatedImage;
}

IplImage *ScaleImage(IplImage *src, const IplImage *imgPrimary, float scale) {
	unsigned int width, height;
	CvPoint2D32f center;

	width = imgPrimary->width;
	height = imgPrimary->height;

	IplImage *scaledImage = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, imgPrimary->nChannels);
	center.x = 0;
	center.y = 0;
	
	CvMat *mapMatrix = cvCreateMat( 2, 3, CV_32FC1 );

	cv2DRotationMatrix(center, 0.0, scale, mapMatrix);
	cvWarpAffine(src, scaledImage, mapMatrix, CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS, cvScalarAll(0));

	cvReleaseImage(&src);
	cvReleaseMat(&mapMatrix);

	return scaledImage;
}

IplImage *AddImages(starMatch *starMatches, unsigned int matchCount, char *primaryImage, char **darkFilename, unsigned char numDarkFiles, char **flatFieldFilename, unsigned char numFlatFieldFiles, char *zeroPixFilename) {	 
	int i,j,k,m;
	IplImage* 	imgPrimary = 0;
	IplImage* 	imgStack = 0;
	IplImage* 	imgTemp=0;
	
	IplImage* 	imgDark = 0;
	IplImage*	imgZeroPix =0;
	IplImage*	imgFlat = 0;

	float		*ffRatios=NULL;
	int	minX=0, maxX=0, minY=0, maxY=0;
	unsigned int stackWidth, stackHeight, stackChannels, stackStep;
	unsigned int stackOffsetX, stackOffsetY;
	unsigned char 	*tempData;
	float		*stackData;
	unsigned int	*normalCounter;

	printf("\nStarting the stacking process...\n");
	/*
	imgPrimary=cvLoadImage(starMatches[0].filePrimary, CV_LOAD_IMAGE_COLOR);
 	if(!imgPrimary){
   		printf("Could not load image file: %s\n", primaryImage);
   		exit(0);
 	}
	*/
	imgPrimary=LoadImage(starMatches[0].filePrimary);

	if(zeroPixFilename) {
		printf("\tGenerating zero second frame...\n");
		imgZeroPix=LoadImage(zeroPixFilename);
	}
	if(darkFilename[0]) {
		printf("\tGenerating dark frame...\n");
		imgDark=AverageImages(darkFilename, numDarkFiles);
		if(imgZeroPix)
			SubImages(imgDark,imgZeroPix);	
	}
	if(flatFieldFilename[0]) {
		printf("\tGenerating flat frame...\n");
		imgFlat=AverageImages(flatFieldFilename, numFlatFieldFiles);
		if(imgZeroPix)
			SubImages(imgFlat,imgZeroPix);	
		if(imgDark)
			SubImages(imgFlat,imgDark);
			ffRatios = malloc(sizeof(float)*imgFlat->width*imgFlat->height*imgFlat->nChannels);
			GenFlatFieldRatio(ffRatios, imgFlat);
	}
	for(i=0;i<matchCount;i++) {
		if(starMatches[i].deltaX < minX) minX = starMatches[i].deltaX;
		if(starMatches[i].deltaX > maxX) maxX = starMatches[i].deltaX;
		if(starMatches[i].deltaY < minY) minY = starMatches[i].deltaY;
		if(starMatches[i].deltaY > maxY) maxY = starMatches[i].deltaY;
	}

	//Allocate the proper size image
	stackWidth = (unsigned int)((int)imgPrimary->width+maxX-minX);
	stackHeight = (unsigned int) ((int)imgPrimary->height+maxY-minY);
	
//	printf("minX:%d\tmaxX:%d\tminY:%d\tmaxY:%d\n",minX,maxX,minY,maxY);	
//	printf("stackWidth:%d\tstackHeight:%d\n",stackWidth,stackHeight);	
	
	imgStack = cvCreateImage(cvSize(stackWidth,stackHeight), IPL_DEPTH_32F,3);
	stackData = (float*)imgStack->imageData;
	stackStep = imgStack->widthStep/sizeof(float);
	stackChannels = imgStack->nChannels;
	
//	printf("stackStep:%d\n",imgStack->widthStep);	

	tempData = (unsigned char *) imgPrimary->imageData;
	stackOffsetX = (unsigned int)abs(minX);
	stackOffsetY = (unsigned int)abs(minY);

	//Initialize normalizing array
	normalCounter = malloc(sizeof(unsigned int)*stackWidth*stackHeight*stackChannels);
	for(i=0;i<stackWidth*stackHeight*stackChannels;i++) {
		normalCounter[i]=0;
	}
	//	printf("i=%d,j=%d,k=%d\n",stackOffsetY,stackOffsetX,stackChannels);	

	printf("Stacking %s\n", starMatches[0].filePrimary);

	if(imgZeroPix) { SubImages(imgPrimary, imgZeroPix); }
	if(imgDark) { SubImages(imgPrimary, imgDark); }
	if(ffRatios) { ApplyFlatField(ffRatios, imgPrimary); }
	for(i=stackOffsetY;i<imgPrimary->height+stackOffsetY;i++) 
		for(j=stackOffsetX;j<imgPrimary->width+stackOffsetX;j++) 
			for(k=0;k<stackChannels;k++) {
				stackData[i*stackStep+j*stackChannels+k]+= (float) tempData[(i-stackOffsetY)*imgPrimary->widthStep+(j-stackOffsetX)*stackChannels+k];
				if(tempData[(i-stackOffsetY)*imgPrimary->widthStep+(j-stackOffsetX)*stackChannels+k]){
					normalCounter[i*stackStep+j*stackChannels+k]++;
				}
			}

	for(m=0;m<matchCount;m++) {
		printf("Stacking %s\n", starMatches[m].file);
		imgTemp=LoadImage(starMatches[m].file);
		
		//transform the image here
//		printf("tempWidth:%d\ttempHeight:%d\trotate:%f\n",imgTemp->width, imgTemp->height, starMatches[m].rotate);	
		if(imgZeroPix) { SubImages(imgTemp, imgZeroPix); }
		if(imgDark) { SubImages(imgTemp, imgDark);}
		if(ffRatios) { ApplyFlatField(ffRatios, imgTemp); }
		imgTemp = RotateImage(imgTemp, imgPrimary, starMatches[m].rotate*(180.0/M_PI));
		imgTemp = ScaleImage(imgTemp, imgPrimary, starMatches[m].scale);
		tempData = (unsigned char *) imgTemp->imageData;
/*
	 	cvNamedWindow("mainWin", 0); 
 		cvMoveWindow("mainWin", 100, 100);
		cvShowImage("mainWin", imgTemp);
		cvWaitKey(0);
*/
		//add the image here
		stackOffsetX = (unsigned int)(starMatches[m].deltaX+abs(minX));
		stackOffsetY = (unsigned int)(starMatches[m].deltaY+abs(minY));
	//	printf("i=%d,j=%d,k=%d\n",stackOffsetY,stackOffsetX,stackChannels);	

  	for(i=stackOffsetY;i<imgTemp->height+stackOffsetY;i++) 
			for(j=stackOffsetX;j<imgTemp->width+stackOffsetX;j++) 
				for(k=0;k<stackChannels;k++) {
					stackData[i*stackStep+j*stackChannels+k]+= (float) tempData[(i-stackOffsetY)*imgTemp->widthStep+(j-stackOffsetX)*stackChannels+k];	
					if(tempData[(i-stackOffsetY)*imgPrimary->widthStep+(j-stackOffsetX)*stackChannels+k]){
						normalCounter[i*stackStep+j*stackChannels+k]++;
					}
				}

		cvReleaseImage(&imgTemp);
	}		
	
	for(i=0;i<stackHeight;i++) 
		for(j=0;j<stackWidth;j++) 
			for(k=0;k<stackChannels;k++) 
				stackData[i*stackStep+j*stackChannels+k] = stackData[i*stackStep+j*stackChannels+k]/(float)normalCounter[i*stackStep+j*stackChannels+k];
				

	cvReleaseImage(&imgPrimary);
	//cvReleaseImage(&imgStack);
	free(normalCounter);
	free(ffRatios);
	return imgStack;
} 

char SubImages(IplImage *a, IplImage *b) {
//	a = a-b
	unsigned int i,j,k;
	unsigned int aWidth, aHeight, aChannels, aStep;
	unsigned int bWidth, bHeight, bChannels, bStep;
	unsigned char *aData;	
	unsigned char *bData;

	aHeight = a->height;
	aWidth = a->width;
	aChannels = a->nChannels;
	aStep = a->widthStep;
	aData = (unsigned char *) a->imageData;

	bHeight = b->height;
	bWidth = b->width;
	bChannels = b->nChannels;
	bStep = b->widthStep;
	bData = (unsigned char *) b->imageData;


	if(aHeight == bHeight)
		if(aWidth == bWidth)
			if(aChannels == bChannels)
				if(aStep == bStep){
					for(i=0;i<aHeight;i++) 
							for(j=0;j<aWidth;j++) 
								for(k=0;k<aChannels;k++) { 
									if(aData[i*aStep+j*aChannels+k] >= bData[i*aStep+j*aChannels+k]) {
										aData[i*aStep+j*aChannels+k] = aData[i*aStep+j*aChannels+k] - bData[i*bStep+j*bChannels+k];
										//printf("aData: %d \t bdata: %d\n",aData[i*aStep+j*aChannels+k],bData[i*bStep+j*bChannels+k]);
									}
									else {
										aData[i*aStep+j*aChannels+k] = 0;
									}
								}
								return 1;
				}
				
	printf("Images must be of same size and color depth in order to subtract.\n");
	return 0; 
}

IplImage *AdjustHistogram(const IplImage *imgIn, float lowHist, float highHist) {
	int i,j,k;
	float *data;	
	uchar *dataOut;
	unsigned int width, height, channels, step;
	float histRatio;
	float histDiff;
	IplImage *img = cvCloneImage(imgIn);
	IplImage *imgOut;
	imgOut = cvCreateImage(cvSize(img->width,img->height),IPL_DEPTH_8U,3);
	
	data = (float*)img->imageData;
	dataOut = (unsigned char*)imgOut->imageData;

	step = img->widthStep/sizeof(float);
	channels = img->nChannels;
	
	histDiff = highHist - lowHist;

	for(i=0;i<img->height;i++) 
		for(j=0;j<img->width;j++) 
			for(k=0;k<img->nChannels;k++) {
				if(data[i*step+j*channels+k] < lowHist) {
					data[i*step+j*channels+k] = 0;
				}
				else if(data[i*step+j*channels+k] > highHist) {
					data[i*step+j*channels+k] = 255;
				}
				else {
					data[i*step+j*channels+k] =  (data[i*step+j*channels+k]-lowHist)*255.0/histDiff;
				}
			}

	for(i=0;i<img->height;i++) 
		for(j=0;j<img->width;j++) 
			for(k=0;k<img->nChannels;k++) 
				dataOut[i*imgOut->widthStep+j*imgOut->nChannels+k] = (unsigned char) roundf(data[i*step+j*channels+k]);

	cvReleaseImage(&img);
	return imgOut;
}

IplImage *AverageImages(char **filenames, unsigned int numFiles) {
	int i,j,k,m;
	uchar *dataOut, *data;
	IplImage *img=0;
	IplImage *imgOut=0;
	
	img=LoadImage(filenames[0]);	
	imgOut = cvCloneImage(img);
	dataOut = (unsigned char*) imgOut->imageData;
	cvReleaseImage(&img);
	
	for(m=1;m<numFiles;m++) {
		img=LoadImage(filenames[m]);	
		data = (unsigned char*) img->imageData;
		for(i=0;i<img->height;i++) 
			for(j=0;j<img->width;j++) 
				for(k=0;k<img->nChannels;k++) {
					dataOut[i*imgOut->widthStep+j*imgOut->nChannels+k] += data[i*img->widthStep+j*img->nChannels+k];
				}
		cvReleaseImage(&img);
	}
	if(numFiles > 1) {	
		for(i=0;i<imgOut->height;i++) 
			for(j=0;j<imgOut->width;j++) 
				for(k=0;k<imgOut->nChannels;k++)
					dataOut[i*imgOut->widthStep+j*imgOut->nChannels+k] /= numFiles; 
	}

	return imgOut;

}

float GenFlatFieldRatio(float *ffRatios, IplImage* img) {
	unsigned int i,j,k;
	unsigned char *data;
	float averagePixel=0;
	data = (unsigned char*) img->imageData;
	for(i=0;i<img->height;i++) 
		for(j=0;j<img->width;j++) 
			for(k=0;k<img->nChannels;k++) 
				averagePixel += data[i*img->widthStep+j*img->nChannels+k];
			
	averagePixel /= (float)(img->height*img->width*img->nChannels);

	for(i=0;i<img->height;i++) 
		for(j=0;j<img->width;j++) 
			for(k=0;k<img->nChannels;k++) { 
				ffRatios[i*img->widthStep+j*img->nChannels+k]=(float)data[i*img->widthStep+j*img->nChannels+k]/averagePixel;
			}
	printf("Average Pixel Value: %f\n", averagePixel);		
	return averagePixel;
}

char ApplyFlatField(float *ffRatios, IplImage *img) {
	unsigned int i,j,k;
	unsigned char *data;
	unsigned int  temp;
	data = (unsigned char*) img->imageData;
	for(i=0;i<img->height;i++) 
		for(j=0;j<img->width;j++) 
			for(k=0;k<img->nChannels;k++) { 
				temp = lroundf((float)data[i*img->widthStep+j*img->nChannels+k] * ffRatios[i*img->widthStep+j*img->nChannels+k]);
				if(temp > 255) {
					data[i*img->widthStep+j*img->nChannels+k] = 255;	
				}
				else {
					data[i*img->widthStep+j*img->nChannels+k] = (unsigned char) temp;
				}
			}
	return 1;
}

IplImage* DrawHistogram(CvHistogram *hist, float scaleX, float scaleY){
	float histMax = 0;
	int numPts = 5;
	CvPoint pt1;
	CvPoint pt2;
	CvPoint pt3;
	CvPoint pt4;
	
	cvGetMinMaxHistValue(hist, 0, &histMax, 0, 0); 
	IplImage* imgHist = cvCreateImage(cvSize(256*scaleX, 64*scaleY), 8, 3);
	cvZero(imgHist);
	for(int i=0;i<255;i++)	{
		float histValue = cvQueryHistValue_1D(hist, i);
		float nextValue = cvQueryHistValue_1D(hist, i+1);
 
		pt1 = cvPoint(i*scaleX, 64*scaleY);
		pt2 = cvPoint(i*scaleX+scaleX, 64*scaleY);
		pt3 = cvPoint(i*scaleX+scaleX, (64-nextValue*64/histMax)*scaleY);
		pt4 = cvPoint(i*scaleX, (64-histValue*64/histMax)*scaleY);
 
		CvPoint pts[] = {pt1, pt2, pt3, pt4, pt1};
 
		cvFillConvexPoly(imgHist, pts, numPts, cvScalar(255,255,255,255), 0, 0);
	}

	return imgHist;
} 
