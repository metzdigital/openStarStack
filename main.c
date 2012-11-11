#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <argp.h>

#include <cv.h>
#include <highgui.h>

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "arguments.h"
#include "pixelLib.h"
#include "starAlign.h"
#include "starStack.h"
#include "fileManage.h"
#include "gui.h"


const char *argp_program_version = 		 "openStack 0.3";
const char *argp_program_bug_address = 	 "<brandonmetz@gmail.com>";

IplImage *imgStack = 0;
IplImage *imgFileOut = 0;

extern numBins;

int main(int argc, char *argv[])
{
	int i,j,k,m;
	starMatch *starMatches;
	unsigned int matchCount=0;

	struct arguments arguments;
	/* Default values. */
	arguments.numFiles = 0;
	arguments.threshold = 75;
	arguments.rssMetric = 0.001;
	arguments.angleMetric = 0.002;
	arguments.minAngle = 0.1;
	arguments.triNStars = 30;
	arguments.primaryImage = '\0';
	arguments.imageOutFilename = "temp.jpg";
	arguments.alignmentFilename = "tempAlign.csv";
	arguments.alignOut = 0;
	arguments.darkFramesFilename[0] = '\0';
	arguments.darkFramesIndex = 0;
	arguments.flatFieldFilename[0] = '\0';
	arguments.flatFieldIndex = 0;
	arguments.zeroPixFilename = '\0';

	int p[3];
  	p[0] = CV_IMWRITE_JPEG_QUALITY;
  	p[1] = 100;
  	p[2] = 0;

	
	/* Parse our arguments; every option seen by parse_opt will
			be reflected in arguments. */
	gtk_init(&argc, &argv);
	argp_parse (&argp, argc, argv, 0, 0, &arguments);

//	LoadAlignmentFromFile("tempAlign.csv", 0x0, 0x0);

       
    	  	
	printf("Using %d dark frame[s]:\n", arguments.darkFramesIndex);
	if(arguments.darkFramesFilename[0] != '\0') {
		for(i=0;i<arguments.darkFramesIndex; i++) {
			printf("\t%s\n",arguments.darkFramesFilename[i]);
		}
	}
	else printf("\t%s\n","none");
	
	printf("Using flat field frame:\n", arguments.darkFramesIndex);
	if(arguments.flatFieldFilename[0] != '\0') {
		for(i=0;i<arguments.flatFieldIndex; i++) {
			printf("\t%s\n",arguments.flatFieldFilename[i]);
		}
	}
	else printf("\t%s\n","none");
	
	printf("Using zero second frame:\n", arguments.darkFramesIndex);
	if(arguments.zeroPixFilename != '\0')
		printf("\t%s\n",arguments.zeroPixFilename);
	else printf("\t%s\n","none");

	if(arguments.primaryImage == '\0') {
		arguments.primaryImage = arguments.file[0];
	}

	starMatches = AlignImages(arguments.alignmentFilename, arguments.primaryImage, arguments.file, arguments.numFiles, &matchCount, arguments.threshold, arguments.rssMetric, arguments.angleMetric, arguments.minAngle, arguments.triNStars);
	//Rotates, scales, and translates each image, combines them, and returns an IplImage pointer to the stacked image
	imgStack = AddImages(starMatches,matchCount,arguments.primaryImage,arguments.darkFramesFilename,arguments.darkFramesIndex,arguments.flatFieldFilename,arguments.flatFieldIndex,arguments.zeroPixFilename);
	
	//Now that images are aligned and stacked, launch the gui to adjust histogram	
	GuiInit("./glade/openStackGUI.glade");	

/*	
	while(1) {
		// create a window		
		switch(cvWaitKey(200)&0xFF){
			case 's':
				printf("Saving to %s\n", arguments.imageOutFilename);
				if(arguments.imageOutFilename) 
					cvSaveImage(arguments.imageOutFilename, imgFileOut, p);
				break;
			case 'e':
				exit(1);
				break;
			case 'u':
				cvReleaseImage(&imgFileOut);
				imgFileOut = AdjustHistogram(imgStack, 255.0/5000.0*(float)lowVal, 255.0/5000.0*(float)highVal);
				cvShowImage("Image", imgFileOut);
				break;
		}

	}

*/
	free(starMatches);
	//free(starObjects1);
	//free(triangles1);
	//cvReleaseImage(&imgPrimary);
  return 0;
}
