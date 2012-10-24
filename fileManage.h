#ifndef _FILEMANAGE_H_
#define _FILEMANAGE_H_

char SaveAlignmentToFile(char*,starMatch*,unsigned int); 
char LoadAlignmentFromFile(char*, starMatch*,unsigned int*);
int  NumLines(char *);
int  CheckFileType(char*);
char ConvertFile(char *);
IplImage *LoadImage(char *);

#endif
