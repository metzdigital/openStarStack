#ifndef __STARSTACK_H_
#define __STARSTACK_H_

#ifndef M_PI
#define M_PI  3.14159265
#endif

IplImage *AddImages(starMatch*,unsigned int,char*,char**,unsigned char,char**,unsigned char,char*); 
char	  SubImages(IplImage*, IplImage*);
IplImage *RotateImage(IplImage*, const IplImage*, float);
IplImage *ScaleImage(IplImage*, const IplImage*, float);
IplImage *AdjustHistogram(const IplImage *, float , float );
IplImage *AverageImages(char **, unsigned int);
float 	  GenFlatFieldRatio(float *, IplImage* );
char 	  ApplyFlatField(float *, IplImage *);
IplImage* DrawHistogram(CvHistogram*, float, float);
#endif
