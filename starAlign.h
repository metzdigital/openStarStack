#ifndef __STARALIGN_H__
	#define __STARALIGN_H__
#define	MIN_STAR_COUNT 15	
#define FALSE 0
#define TRUE  1

typedef struct _triangleFeature {
	imgObject		*obj[3];
	float			angle[3];
	float			metric;
	char 			objMatch[3];
	unsigned int	index[3];
} triangleFeature;

typedef struct _possibleMatch {
	triangleFeature *tri1;
	triangleFeature *tri2;
} possibleMatch;

typedef struct _starMatch {
	char file[2048];
	char filePrimary[2048];
//	triangleFeature	*trianglePrimary;
//	triangleFeature	*triangleMatch;
	float scale;
	float rotate;
	int deltaX;
	int deltaY;	
} starMatch;

unsigned int TriangulateFeatures(imgObject*, unsigned int, triangleFeature*); 
imgObject* FindAndClassifyStars(IplImage*, unsigned int*, unsigned int);
float UnitDotProduct(float,float,float,float);
int	CrossProduct(float , float , float , float);
float	Mag(float a, float b);
float FindAngle(float , float, float, float, float, float); 
char MatchObjects(triangleFeature	*, triangleFeature *, unsigned int , float , float, float, unsigned int *, unsigned int *, possibleMatch *, unsigned int*);
char CheckMassOrder(triangleFeature, triangleFeature); 
int LogMatch(char* , char*, starMatch*, starMatch); 
starMatch* AlignImages(char*, char*, char** , unsigned int , unsigned int *, unsigned char , float , float , float, unsigned int);
char AuditMatches(possibleMatch*, unsigned int, starMatch*);
char TransformMatch(triangleFeature *, triangleFeature *, float *, float *, float *, float *);
double factorial(unsigned int);
double nchoosek(unsigned int, unsigned int);
#endif
