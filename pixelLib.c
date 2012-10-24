#include <cv.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pixelLib.h"


void InitPixelArray(pixel *image, unsigned int size) {
	unsigned int i;
	
	for(i=0;i<size;i++) {
		image[i].x=0xFFFFFFFF;
		image[i].y=0xFFFFFFFF;
		image[i].mag=0xFFFFFFFF;
		image[i].objectID = NULL;
		
	}
	for(i=0; i<MAX_OBJ_ITEMS; i++) {
		objectIDArray[i] = i;
	}
}

void InitObjectArray(imgObject *object, unsigned int size) {
	unsigned int i;
	for(i=0;i<size;i++) {
		object[i].x=0;
		object[i].y=0;
		object[i].width=0;
		object[i].height=0;
		object[i].mass=0;
		object[i].numberItems=0;
		object[i].objectID = 0;
		object[i].score = 0;
	}
}

unsigned int *SearchPixels(pixel *image, unsigned int size, unsigned int x, unsigned int y, unsigned int width) {
	/*unsigned int i;
	for(i=0;i<size;i++) {
		if(image[i].objectID == NULL) {
			return -1;
		}
		if(image[i].x == x && image[i].y == y) {
			return i;
		}
	}
	*/
	if(image[y*width+x].objectID == NULL) {
		return NULL;
	}
	else {
		return image[y*width+x].objectID;
	}
}

unsigned int ClassifyPixels(pixel *image, unsigned int size,  IplImage* imageSource, unsigned int *pixelsFound) {
	int i,j;
	unsigned int height;
	unsigned int width;
	unsigned int widthStep;
	unsigned char *data;
	unsigned int *checkObjID;
	unsigned int pixelCount = 0;
	unsigned int pixelIndex = 0;
	unsigned int objectArrayIndex = 0;

	height = imageSource->height;
	width = imageSource->width;
	widthStep = imageSource->widthStep;
	data = (unsigned char *) imageSource->imageData;


	for(i=1; i<height; i++) {
		for(j=1;j<width; j++) {
			if(data[i*widthStep+j] != 0) {
				//Create a new pixel instance
				image[i*width+j].x = j;
				image[i*width+j].y = i;
				image[i*width+j].mag = data[i*widthStep+j];
				//Search for neighbors that have an objectID
				//Check for a pixel above
				checkObjID = SearchPixels(image, size, j, i-1,width);
				if(checkObjID != NULL) {
					//printf("UP ObjectID = %d\n", *checkObjID);
					image[i*width+j].objectID = checkObjID;
					//See if there was a pixel to the left of it, and if so, change its objectID to the one above
					checkObjID = SearchPixels(image,size,j-1,i,width);
					if(checkObjID != NULL) {
						*image[i*width+(j-1)].objectID = *image[i*width+j].objectID;
					}
				}
				else {
					//no top pix found, check the left pixel
					checkObjID = SearchPixels(image, size, j-1, i,width);
					if(checkObjID != NULL) {	
						//printf("LEFT ObjectID = %d\n", *checkObjID);
						image[i*width+j].objectID = checkObjID;
					}
					else {
						//No nearby objects found, create new objectID
						image[i*width+j].objectID = &objectIDArray[objectArrayIndex];
						//printf("NEW ObjectID = %d\n", *image[i*width+j].objectID);
						objectArrayIndex++;	
					}
				}

				pixelCount++;
			}
		} 
	}

/*
	for(i=0; i<objectArrayIndex; i++) {
		printf("ObjectIDArray[%d] = %d\n", i, objectIDArray[i]);
	}
*/

	if(objectArrayIndex == 0)
		objectArrayIndex = 1;
	
	*pixelsFound = pixelCount;
	return objectIDArray[objectArrayIndex-1];
}                                                             

void	CleanUpObjects(pixel *image, pixel *imageReduced, unsigned int size, unsigned int pixelCount) {
	unsigned int i;
	unsigned int pixelIndex=0;

	for(i=0; i<size && pixelIndex < pixelCount; i++) {
		if(image[i].objectID == NULL) {}
		else {
			imageReduced[pixelIndex++] = image[i];
		}
	}	
	/*		
	for(i=0; i<pixelCount; i++) {
		printf("X: %d, Y: %d, mag: %d, objectID %d\n",image[i].x,image[i].y,image[i].mag, *image[i].objectID);
	}
	*/
}
unsigned int ClassifyObjects(imgObject* imageObject, unsigned int objectSize, pixel* image, unsigned int pixelSize, IplImage *img){
	unsigned int i,j;
	unsigned int objectsFound=0;
	unsigned int objectCounter=0;
	unsigned int minX =0xFFFFFFFF;
	unsigned int minY =0xFFFFFFFF;
	unsigned int maxX =0;
	unsigned int maxY =0;
	float tempX =0;
	float tempY =0;
	unsigned int tempMass =0;
	
	for(i=0; i<=objectSize; i++) {
		for(j=0; j<pixelSize; j++) {
			if(*image[j].objectID == i) {
				tempX += (float) image[j].x * (float) image[j].mag;
				tempY += (float) image[j].y * (float) image[j].mag;
				tempMass += image[j].mag;

				objectsFound++;
				//Used to calculate the width and height
				if(image[j].x < minX) minX = image[j].x;
				if(image[j].x > maxX) maxX = image[j].x;
				if(image[j].y < minY) minY = image[j].y;
				if(image[j].y > maxY) maxY = image[j].y;
				//We only want stars, not nebula stuff
				if(maxX-minX+1 > 100)
					break;
				if(maxY-minY+1 > 100)
					break;
			}
		}
		if(objectsFound > 5) {
			if(maxX-minX+1 < 100) {
				if(maxY-minY+1 < 100) {	
					//stars should be round, make sure we only use stars	
					if(abs((maxX-minX+1) - (maxY-minY+1)) < 5) {
						imageObject[objectCounter].numberItems = objectsFound;
						imageObject[objectCounter].x = tempX/(float)tempMass; 
						imageObject[objectCounter].y = tempY/(float)tempMass;
						imageObject[objectCounter].width = maxX - minX +1;
						imageObject[objectCounter].height = maxY - minY +1;
						imageObject[objectCounter].mass = tempMass;
						imageObject[objectCounter].objectID = objectCounter;
						imageObject[objectCounter].img = img;
						objectCounter++;
					}
				}
			}
		}
		
		minX =0xFFFFFFFF;
		minY =0xFFFFFFFF;
		maxX =0;
		maxY =0;
		tempX =0;
		tempY =0;
		tempMass =0;
		objectsFound = 0;
	}



	return objectCounter;
}

void RankObjects(imgObject *imageObjects, unsigned int size) {
	int i,j;
	float temp;

	for(i=0;i<size;i++) {
		for(j=0; j<size; j++) {
			if(i==j) {
		
			}
			else {
				imageObjects[i].score += imageObjects[i].mass*imageObjects[j].mass/sqrt(pow((signed)imageObjects[i].x-(signed)imageObjects[j].x,2)+ pow((signed)imageObjects[i].y-(signed)imageObjects[j].y,2));
			}		
		}
	}	
	
	for(i=0;i<size;i++) {
		for(j=0; j<size-1; j++) {
			if(imageObjects[j].score < imageObjects[j+1].score) {
				SwapObjects(&imageObjects[j], &imageObjects[j+1]);
			}
		}
	}
/*	
	for(i=0; i<size; i++) {
		printf("|| ObjID: %d || x: %f || y: %f || width: %d || height: %d || mass: %d || score: %f \n ", imageObjects[i].objectID, imageObjects[i].x, imageObjects[i].y, imageObjects[i].width, imageObjects[i].height,	imageObjects[i].mass, imageObjects[i].score);
	}
*/
}

void SwapObjects(imgObject *obj1, imgObject *obj2) {
	imgObject objTemp;

	objTemp.numberItems = obj2->numberItems;
	objTemp.x = obj2->x;
	objTemp.y = obj2->y;
	objTemp.width = obj2->width;
	objTemp.height = obj2->height;
	objTemp.mass = obj2->mass;
	objTemp.objectID = obj2->objectID;
	objTemp.score = obj2->score;
	
	obj2->numberItems = obj1->numberItems;
	obj2->x = obj1->x;
	obj2->y = obj1->y;
	obj2->width = obj1->width;
	obj2->height = obj1->height;
	obj2->mass = obj1->mass;
	obj2->objectID = obj1->objectID;
	obj2->score = obj1->score;

	obj1->numberItems = objTemp.numberItems;
	obj1->x = objTemp.x;
	obj1->y = objTemp.y;
	obj1->width = objTemp.width;
	obj1->height = objTemp.height;
	obj1->mass = objTemp.mass;
	obj1->objectID = objTemp.objectID;
	obj1->score = objTemp.score;
}

