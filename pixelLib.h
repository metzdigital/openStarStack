#ifndef __PIXELLIB_H__
	#define __PIXELLIB_H__


#define MAX_OBJ_ITEMS 256*256

typedef struct _pixel {
	unsigned int x;
	unsigned int y;
	unsigned int mag;
	unsigned int *objectID;
} pixel;

typedef struct _imgObject {
	float x;
	float y;
	unsigned int width;
	unsigned int height;
	unsigned int mass;
	unsigned int numberItems;
	unsigned int objectID;
	float 		 score;
	IplImage	*img;
} imgObject;

typedef struct _objRelation {
	imgObject 	*HeadObject;
	imgObject 	*TailObject;
	int			xComp;
	int			yComp;
	float 		distance;
} objRelation;

unsigned int pixelIndex;

unsigned int objectIDArray[MAX_OBJ_ITEMS];

void InitPixelArray(pixel*, unsigned int);
void InitObjectArray(imgObject*, unsigned int);
//returns the number of pixels found
unsigned int ClassifyPixels(pixel*, unsigned int, IplImage*, unsigned int*);
//returned the pixel index that contains the x,y pixel you're looking for
unsigned int *SearchPixels(pixel*, unsigned int, unsigned int, unsigned int, unsigned int);
void	CleanUpObjects(pixel *, pixel *, unsigned int, unsigned int );
unsigned int ClassifyObjects(imgObject*, unsigned int, pixel*, unsigned int, IplImage*);
void RankObjects(imgObject*, unsigned int);
void SwapObjects(imgObject*, imgObject*);

#endif
