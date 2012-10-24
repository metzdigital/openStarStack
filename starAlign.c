#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <math.h>
#include "pixelLib.h" 
#include "starAlign.h"
#include "fileManage.h"

imgObject* FindAndClassifyStars(IplImage *imgSrc, unsigned int *starCountMain,  unsigned int threshold) {
	unsigned int width, height;
	unsigned int imageSize;
	unsigned int pixelsFound;
	unsigned int starCount;
	IplImage *imgGray;
	pixel *image;
	pixel *imageReduced;
	imgObject *starObjects;

	width = imgSrc->width;
	height = imgSrc->height;
 
	imgGray = cvCreateImage(cvSize(width,height), IPL_DEPTH_8U, 1);

	cvCvtColor(imgSrc, imgGray, CV_RGB2GRAY);

  	//Setup a threshold 
  	cvThreshold(imgGray, imgGray, threshold, 0, CV_THRESH_TOZERO);

/*		
	cvNamedWindow("Gray Image", 0); 
  	cvMoveWindow("Gray Image", 10, 10);
	cvShowImage("Gray Image", imgGray);
	cvWaitKey(0);
*/
	
	//Set the image size
	imageSize = imgGray->height * imgGray->width;

	//Allocate the pixels needed	
	image = (pixel*) malloc( sizeof(pixel) * imageSize);	

	//init the pxel array
	InitPixelArray(image, imageSize);

	//Classify Pixels and return the number of objects found
	printf("\t\tClassifying pixels...\n");
	starCount = ClassifyPixels(image, imageSize, imgGray, &pixelsFound);
	
	imageReduced = (pixel*) malloc( sizeof(pixel) * pixelsFound);
	
	printf("\t\tCleaning objects...\n");
	CleanUpObjects(image, imageReduced, imageSize, pixelsFound);
	free(image);
	
	//Allocate the number of objects needed (kind of)
	starObjects = (imgObject*) malloc( sizeof(imgObject) * starCount);

	//Init the object array
	InitObjectArray(starObjects, starCount);

	//Now organize the objects found
	printf("\t\tOrganizing objects...\n");
	starCount = ClassifyObjects(starObjects, starCount, imageReduced, pixelsFound, imgSrc);

	//Find the score and sort them
	printf("\t\tRanking objects...\n");
	RankObjects(starObjects, starCount);

	*starCountMain = starCount;
  	
	cvReleaseImage(&imgGray);
	free(imageReduced);
	return starObjects;
}

unsigned int TriangulateFeatures(imgObject *objectSet, unsigned int starCount, triangleFeature *triangle) {
	unsigned int i_start = 1;
	unsigned int j_start = 2;
	unsigned int k_start = 3;
	unsigned int j_mid = j_start;
	unsigned int k_mid = k_start;
	unsigned int i,j,k;
	unsigned int index = 0;

	if(starCount < 3) {
		return 0;
	}
	for(i = i_start; i<=starCount-2; i++) {
		for(j = j_mid; j<=starCount-1; j++) {
			for(k = k_mid; k<=starCount; k++) {
				
				triangle[index].obj[0] = &objectSet[i-1];	
				triangle[index].obj[1] = &objectSet[j-1];	
				triangle[index].obj[2] = &objectSet[k-1];	
				
				triangle[index].angle[0] = FindAngle(objectSet[i-1].x, objectSet[i-1].y, objectSet[j-1].x, objectSet[j-1].y, objectSet[k-1].x, objectSet[k-1].y); 			
				triangle[index].angle[1] = FindAngle(objectSet[j-1].x, objectSet[j-1].y, objectSet[i-1].x, objectSet[i-1].y, objectSet[k-1].x, objectSet[k-1].y);				
				triangle[index].angle[2] = FindAngle(objectSet[k-1].x, objectSet[k-1].y, objectSet[i-1].x, objectSet[i-1].y, objectSet[j-1].x, objectSet[j-1].y); 				
				triangle[index].metric = sqrt(pow(triangle[index].angle[0],2)+pow(triangle[index].angle[1],2)+pow(triangle[index].angle[2],2)); 
				
				triangle[index].index[0] = i;			
				triangle[index].index[1] = j;			
				triangle[index].index[2] = k;			
//				printf("%d\t%d\t%d\t%.3f\t%.3f\t%.3f\t", i,j,k,triangle[index].angle1,triangle[index].angle2,triangle[index].angle3);
//				printf("metric:%f\n",triangle[index].metric);
				
				index++;
			}
			k_mid++;
		}
		j_mid++;
		k_start++;
		k_mid = k_start;
	}
	return index;
}

// Find the angle between three points
float FindAngle(float x1, float y1, float x2, float y2, float x3, float y3) {

	float unitV1[2];
	float unitV2[2];
	float mag1, mag2;

	unitV1[0] = x2-x1;
	unitV1[1] = y2-y1;

	unitV2[0] = x3-x1;
	unitV2[1] = y3-y1;
	
	return acos(UnitDotProduct(unitV1[0],unitV1[1],unitV2[0],unitV2[1]));	
}	

// Find the angle between three points
float UnitDotProduct(float V1i, float V1j, float V2i, float V2j) {

	float mag1, mag2;
	float unitV1i, unitV1j, unitV2i, unitV2j;

	mag1 = sqrt(pow(V1i,2) + pow(V1j,2));
	mag2 = sqrt(pow(V2i,2) + pow(V2j,2));

	unitV1i		= V1i/mag1;
	unitV1j		= V1j/mag1;
	unitV2i		= V2i/mag2;
	unitV2j		= V2j/mag2;
	
	return (unitV1i*unitV2i+unitV1j*unitV2j);	
	
}

int	CrossProduct(float v1i, float v1j, float v2i, float v2j)	{
	// | i 		j 		k |
	// | v1i	v1j		0 |
	// | v2i	v2j		0 |
	// result = k * ((v1i*v2j) - (v1j*v2i))

	return (v1i*v2j)-(v1j*v2i);	
}

float	Mag(float a, float b)	{
	return sqrt(pow(a,2)+(pow(b,2)));
}

char MatchObjects(triangleFeature	*triangleSet1, triangleFeature *triangleSet2, unsigned int indexSize, float metricCriteria, float angleCriteria, float minAngle, unsigned int *matchIndexI, unsigned int *matchIndexJ, possibleMatch *possibleMatches, unsigned int *matches) {
	unsigned int i, j, m, n;
	float 	minValue = 100.0;
	float	angleA, angleB;
	float	minAngleDiff[3] = {100.0, 100.0, 100.0};
	char	objMatchTemp[3] = {-1, -1, -1};

	for(i=0; i<indexSize; i++) {
		for(j=0; j<indexSize; j++) {
			//Check the metric
			/*(fabs(triangleSet1[i].metric-triangleSet2[j].metric) < minValue) && */
			if((fabs(triangleSet1[i].metric-triangleSet2[j].metric) < metricCriteria)) {
				//Make sure its an actual triangle and not a straight line
				if(triangleSet1[i].angle[0] > minAngle && triangleSet1[i].angle[1] > minAngle && triangleSet1[i].angle[2] > minAngle) {
					//If it passes the metric, is an actual triangle (not straight line), then lets check all the angles and see if they agree
					for(m=0; m<3; m++){
						for(n=0;n<3;n++) {
							angleA = triangleSet1[i].angle[m];
							angleB = triangleSet2[j].angle[n];
							if(fabs(angleA-angleB) < minAngleDiff[m]) {
								minAngleDiff[m] = fabs(angleA-angleB);
								triangleSet1[i].objMatch[m] = n;
								triangleSet2[j].objMatch[m] = n;		
							}						
//						printf("AngleA = %f\tAngleB = %f\tminAngleDiff[%d] = %f\n",angleA,angleB,m,minAngleDiff[m]);
						}
					}
				}
				//If the angles are correct to within x angleCriteria make them the current most likely match
				if((minAngleDiff[0] < angleCriteria) && (minAngleDiff[1] < angleCriteria) && (minAngleDiff[2] < angleCriteria)) {
//					printf("minAng[0]:%f minAng[1]:%f minAng[2]:%f\n",minAngleDiff[0],minAngleDiff[1],minAngleDiff[2]);


					//Perform the mass/angle check to ensure each angle corresponds with the proper mass size
					if(CheckMassOrder(triangleSet1[i], triangleSet2[j])) {	
						minValue = fabs(triangleSet1[i].metric-triangleSet2[j].metric);
						*matchIndexI = i;
						*matchIndexJ = j;
	//					printf("Min value: %f\n", minValue);
						//Save the possible valid match

						possibleMatches[(*matches)].tri1 = &triangleSet1[i];
						possibleMatches[(*matches)].tri2 = &triangleSet2[j];
						(*matches)++;
					}
				}
				minAngleDiff[0] = 100.0; 
				minAngleDiff[1] = 100.0; 
				minAngleDiff[2] = 100.0; 
			}
/*			
			if(fabs(triangleSet1[i].metric-triangleSet2[j].metric) < metricCriteria) {
				printf("metricCriteria met: %f\n", fabs(triangleSet1[i].metric-triangleSet2[j].metric));
			}
			printf("%.3f\t%.3f\t%.3f\t", triangleSet1[i].angle[0],triangleSet1[i].angle[1],triangleSet1[i].angle[2]);
			printf("metric:%f\n",triangleSet1[i].metric);
			printf("%.3f\t%.3f\t%.3f\t", triangleSet2[j].angle[0],triangleSet2[j].angle[1],triangleSet2[j].angle[2]);
			printf("metric:%f\n",triangleSet2[j].metric);
			printf("diff: %f\n\n",	fabs(triangleSet1[i].metric - triangleSet2[j].metric));
*/
		}
	}


	//printf("Matches: %d\n", matches);
	if(minValue < metricCriteria) {
		return 1; 
	}

	return 0;
}

char CheckMassOrder(triangleFeature triangle1, triangleFeature triangle2) {
	unsigned int i, j;
	unsigned int mass1[3] = {0,0,0};
	unsigned int mass2[3] = {0,0,0};
	unsigned int tempMass;
	char		 massOrder1[3] = {0,1,2};
	char		 massOrder2[3] = {0,1,2};
	char		 tempOrder;
	char		 validMatchFlag = FALSE;
	
	for(i=0;i<3;i++){
		mass1[i] = triangle1.obj[i]->mass;
		mass2[i] = triangle2.obj[i]->mass; 
	}	

	//sort them and keep track
	for(i=0;i<3;i++) {
		for(j=0;j<3;j++) {
			if(mass1[i]>mass1[j]) {
				tempMass = mass1[i];
				mass1[i] = mass1[j];
				mass1[j] = tempMass;
				tempOrder = massOrder1[i];
				massOrder1[i] = massOrder1[j];
				massOrder1[j] = tempOrder;
			}
			if(mass2[i]>mass2[j]) {
				tempMass = mass2[i];
				mass2[i] = mass2[j];
				mass2[j] = tempMass;
				tempOrder = massOrder2[i];
				massOrder2[i] = massOrder2[j];
				massOrder2[j] = tempOrder;
			}
		}
	}
/*
	for(i=0;i<3;i++) {
		printf("%d: massOrder1 = %d\tmassOrder2 = %d\n", i, massOrder1[i], massOrder2[i]);
	}
	for(i=0;i<3;i++) {
		printf("%d: mass1 = %d\tmass2 = %d\n", i, mass1[i], mass2[i]);
	}
*/
	for(i=0;i<3;i++){
		if(triangle1.objMatch[massOrder1[i]] == triangle2.objMatch[massOrder2[i]]) {	
//			printf("Match\n");
		}
		else {
			return 0;
		}
	}
	
	return 1;
}

int LogMatch(char* filename, char *filenamePrimary, starMatch *match, starMatch tempMatch) {
	printf("%s\t%d\t%d\t%f\t%f\n",filename,tempMatch.deltaX,tempMatch.deltaY,tempMatch.rotate,tempMatch.scale);
	strcpy(match->file,filename);
	strcpy(match->filePrimary,filenamePrimary);
	match->scale = tempMatch.scale;
	match->rotate = tempMatch.rotate;
	match->deltaX = tempMatch.deltaX;
	match->deltaY = tempMatch.deltaY;
}

//returns the number of star matches
starMatch* AlignImages(char *alignFilename, char* primaryImage, char** file, unsigned int numFiles, unsigned int *matchCount, unsigned char threshold, float rssMetric, float angleMetric, float minAngle, unsigned int triNStars) {
	IplImage* imgPrimary = 0;
	IplImage* imgFileOut = 0;
	IplImage* imgN = 0;
	int i,j,k,m;
	char filenamePrimary[2048];	
	char filenameN[2048];	
	imgObject *starObjects1, *starObjects2;
	unsigned int starCount1, starCount2;
	CvPoint centerOfObject;

	triangleFeature *triangles1, *triangles2;
	unsigned int triangleCount1, triangleCount2;
	unsigned int triangleMatch1, triangleMatch2;
	CvPoint linep1, linep2;
	
	possibleMatch *possibleMatches;
	unsigned int matches=0;
	starMatch *starMatches;
	starMatch tempMatch;


	printf("Primary image:\n\t%s\n", primaryImage);
	printf("\nStarting the alignment process...\n");
	printf("Filename\tdeltaX\tdeltaY\trotation\tscale\n");		
 	// load image1  
	strcpy(filenamePrimary, primaryImage);
	ConvertFile(filenamePrimary); 
	//strcpy(primaryImage, filenamePrimary);
 	imgPrimary=cvLoadImage(filenamePrimary, CV_LOAD_IMAGE_COLOR);
 	if(!imgPrimary){
   		printf("Could not load image file: %s\n",file[0]);
   		exit(0);
 	}
	starObjects1 = FindAndClassifyStars(imgPrimary, &starCount1, threshold);	
	triangles1 = (triangleFeature*) malloc( sizeof(triangleFeature) * lround(nchoosek(triNStars,3)));
	triangleCount1 = TriangulateFeatures(starObjects1, (triNStars > starCount1) ? starCount1 : triNStars, triangles1);
	starMatches = (starMatch*) malloc( sizeof(starMatch) * numFiles);

	for(i=0; i<numFiles; i++) {
		// load image n
		if(!strcmp(file[i], primaryImage)) {
		}
		else {
			strcpy(filenameN, file[i]);
			ConvertFile(filenameN); 
			imgN=cvLoadImage(filenameN, CV_LOAD_IMAGE_COLOR);
			if(!imgN){
				printf("Could not load image file: %s\n",file[i]);
				exit(0);
			}

			starObjects2 = FindAndClassifyStars(imgN, &starCount2, threshold);	
			triangles2 = (triangleFeature*) malloc( sizeof(triangleFeature) * lround(nchoosek(triNStars,3)));
			printf("\t\tTriangulating objects...\n");
			triangleCount2 = TriangulateFeatures(starObjects2, (triNStars > starCount2) ? starCount2 : triNStars, triangles2);
			possibleMatches = (possibleMatch*) malloc(sizeof(possibleMatch)*triangleCount1);
			matches=0;
			//If valid match found
			printf("\t\tMatching Triangulated objects...\n");
			if(MatchObjects(triangles1, triangles2, triangleCount1, rssMetric, angleMetric, minAngle, &triangleMatch1, &triangleMatch2, possibleMatches, &matches)) {
				if(AuditMatches(possibleMatches, matches, &tempMatch)) {	 
					LogMatch(filenameN, filenamePrimary, &starMatches[(*matchCount)++], tempMatch);
				}
			}
			// release the image
			cvReleaseImage(&imgN);
			//Was getting a seg fault when free-ing these, not sure why, memory usage doesnt seem to go up with these commented out. Oh well.
			//free(starObjects2);
			//free(triangles2);
		}
	}

	SaveAlignmentToFile(alignFilename, starMatches, *matchCount);

	free(starObjects1);
	free(triangles1);
	cvReleaseImage(&imgPrimary);
	return starMatches;
}

char AuditMatches(possibleMatch *possibleMatches, unsigned int matchCount, starMatch *finalMatch) {
	int i,j,k;	
	float scale1, scale2;
	float angle1, angle2;
	float deltaX1,deltaY1,deltaX2,deltaY2;
	unsigned int numMatches=1;
	unsigned int maxMatches=0;
	float avgScale=0, avgAngle=0, avgDeltaX=0, avgDeltaY=0;
	float sumScale=0, sumAngle=0, sumDeltaX=0, sumDeltaY=0;
	
	for(i=0;i<matchCount;i++){
		TransformMatch(possibleMatches[i].tri1,possibleMatches[i].tri2,&scale1,&angle1,&deltaX1,&deltaY1);
		sumScale += scale1;
		sumAngle += angle1;
		sumDeltaX += deltaX1;
		sumDeltaY += deltaY1;
		for(j=0;j<matchCount;j++) {
			if(i>=j){}
			else {
				TransformMatch(possibleMatches[j].tri1,possibleMatches[j].tri2,&scale2,&angle2,&deltaX2,&deltaY2);
				if(fabs(scale1-scale2) < 0.01)
					if(fabs(angle1-angle2) < 0.01)
						if(fabs(deltaX1-deltaX2) < 1)
							if(fabs(deltaY1-deltaY2) < 1){
							//	printf("(%d,%d): scale1:%f vs scale2:%f\t\trot1:%f vs rot2:%f\t\tdx1:%f dx2:%f\n",i,j,scale1,scale2,angle1,angle2,deltaX1,deltaX2);
								numMatches++;
								sumScale += scale2;
								sumAngle += angle2;
								sumDeltaX += deltaX2;
								sumDeltaY += deltaY2;
							}
				
			}
		}
		if(numMatches > maxMatches) {
			avgScale = sumScale/(float)numMatches;
			avgAngle = sumAngle/(float)numMatches;
			avgDeltaX = sumDeltaX/(float)numMatches;
			avgDeltaY = sumDeltaY/(float)numMatches;
			maxMatches = numMatches;
		}
		numMatches =1;
		sumScale = 0;
		sumAngle = 0;
		sumDeltaX = 0;
		sumDeltaY = 0;
	}
//	printf("matches:%d scale:%f rot:%f dx:%f dy:%f\n",maxMatches, avgScale,avgAngle,avgDeltaX,avgDeltaY);

	finalMatch->file[0] = '\0';
	finalMatch->scale = avgScale;
	finalMatch->rotate = avgAngle;
	finalMatch->deltaX = roundf(avgDeltaX);
	finalMatch->deltaY = roundf(avgDeltaY);

	if(maxMatches < 3) {
		return 0;
	}
	else {
		return 1;
	}

}

char TransformMatch(triangleFeature *tri1, triangleFeature *tri2, float *scale, float *rot, float *deltaX, float *deltaY) {
	float	xNew, yNew;
	float	x1Temp, y1Temp, x2Temp, y2Temp;
	float	x1Center, y1Center, x2Center, y2Center;
	float	v1i,v1j,v2i,v2j;
	float 	mag2;	
	float	ang2;

	v1i = tri1->obj[1]->x - tri1->obj[0]->x;
	v1j = tri1->obj[1]->y - tri1->obj[0]->y;
	v2i = tri2->obj[tri1->objMatch[1]]->x - tri2->obj[tri1->objMatch[0]]->x;
	v2j = tri2->obj[tri1->objMatch[1]]->y - tri2->obj[tri1->objMatch[0]]->y;
		
	//Rotation
	*rot = acos(UnitDotProduct(v1i,v1j,v2i,v2j));
	*rot = (CrossProduct(v1i,v1j,v2i,v2j) < 0) ? -1*(*rot) : (*rot);	
	if(*rot != *rot) { *rot = 0.0; }	
			
	//Scale 
	*scale = sqrt(pow(v1i,2) + pow(v1j,2))/sqrt(pow(v2i,2) + pow(v2j,2));

	x1Center = (float)(tri1->obj[0]->img->width/2.0);
	y1Center = (float)(tri1->obj[0]->img->height/2.0);
	x2Center = (float)(tri2->obj[0]->img->width/2.0);
	y2Center = (float)(tri2->obj[0]->img->height/2.0);

	x1Temp = tri1->obj[0]->x;
	y1Temp = tri1->obj[0]->y;
	x2Temp = tri2->obj[tri1->objMatch[0]]->x;
	y2Temp = tri2->obj[tri1->objMatch[0]]->y;
			
	ang2 = atan2(x2Temp-x2Center,y2Temp-y2Center);
	mag2 = Mag(x2Temp - x2Center, y2Temp - y2Center);

	//Need to add back in the center of the image so the translation is with respect to the upper left corner or (0,0)
	xNew = (mag2*(*scale)*sin(ang2+(*rot))) + ((*scale)*x2Center);	
	yNew = (mag2*(*scale)*cos(ang2+(*rot))) + ((*scale)*y2Center);	
				
	//Translation
	*deltaX = -1*(xNew - (x1Temp));
	*deltaY = -1*(yNew - (y1Temp));

	return 1;
}

double factorial(unsigned int n) {
	unsigned int i;
	double temp;
	temp = 1;
	for(i=n; i>0; i--)
		temp*=i;
	return temp;	
}

double nchoosek(unsigned int n, unsigned int k) {
	return factorial(n)/(factorial(k)*factorial(n-k));
}
