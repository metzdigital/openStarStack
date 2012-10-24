#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cv.h>
#include <highgui.h>

#include "pixelLib.h"
#include "starAlign.h"
#include "fileManage.h"


char SaveAlignmentToFile(char *filename, starMatch *starMatches, unsigned int numMatches) {
	int i,j,k;
	FILE *file;
	file = fopen(filename, "w");
	if(!file) {
		printf("Error writing to %s, no parameters will be saved\n",filename);
		return 0;
	}

	fprintf(file,"filename,primaryFile,deltaX,deltaY,rotate,scale\n");
	for(i=0;i<numMatches;i++) {
		fprintf(file,"%s,%s,%d,%d,%f,%f\n",starMatches[i].file,starMatches[i].filePrimary,starMatches[i].deltaX,starMatches[i].deltaY,starMatches[i].rotate,starMatches[i].scale);
	}

	fclose(file);
	return 1;
}

char LoadAlignmentFromFile(char* filename, starMatch* starMatches, unsigned int* matches) {
	int i,j,k;
	FILE *file;
	unsigned char cLine[2048];
	unsigned char cp[2048], *token;
	const	unsigned char delimiters[] = ",";
	int numLines;	

	//grab the number of lines in the file
	numLines = NumLines(filename);
	
	file = fopen(filename, "r");
	if(!file) {
		printf("Error reading %s\n",filename);
		return 0;
	}
	
	printf("Found %d lines\n", numLines);

	//Grab the header line
	fgets(cLine, 0x7FFF, file);
	printf("%s",cLine); 

	for(i=0; i<numLines-1; i++) {
		fgets(cLine, 0x7FFF, file);
		strcpy(cp, cLine);
		token = strtok(cp, delimiters);
		while(token!=NULL) {
			printf("%s\t", token);
			token = strtok(NULL, delimiters);
		}
		printf("\n");
	}
}

int NumLines(char *filename) {
	FILE *f;
	char c;
	int lines = 0;

	f = fopen(filename, "r");

	if(f == NULL)
		return 0;

	while((c = fgetc(f)) != EOF)
		if(c == '\n')
			lines++;

	fclose(f);

	return lines;
}

char ConvertFile(char *filename) {
	char command[2048];
	char *pTemp;
	char *pPlace;
	int i;


	if(CheckFileType(filename)) {
		return 0;
	}	
	else {
		printf("Converting RAW file %s\n", filename);
		sprintf(command, "dcraw -w -T -W \'%s\'", filename);
		system(command);
		pPlace = strrchr(filename,'.');
		strcpy(pPlace,".tiff");
		return 1;
	}
}

int CheckFileType(char* filename) {
	char types[][5] = {".bmp",".png",".jpg",".tiff"};
	char *pos;
	int i;
	for(i=0;i<4;i++) {
		pos = strstr(filename, types[i]);
		if(pos != NULL) {
			return 1;
		}
	}
	return 0;
}

IplImage *LoadImage(char *filename) {
	char filenameCopy[1024];	
	IplImage *img;
	// load image1  
	strcpy(filenameCopy, filename);
	ConvertFile(filenameCopy); 
 	img=cvLoadImage(filenameCopy, CV_LOAD_IMAGE_COLOR);
 	if(!img){
   		printf("Could not load image file: %s\n",filenameCopy);
   		exit(0);
 	}
	
	return img;
}
