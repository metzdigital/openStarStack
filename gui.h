
#ifndef __GUI_H__
#define __GUI_H__

//The globals
IplImage 	*gtkMask;
IplImage 	*imgStack8bit;

GtkImage	*imageMain;
GdkPixbuf	*pixbufMain;

//Histogram stuff
GtkImage	*imageRedHist;
GtkImage	*imageGreenHist;
GtkImage	*imageBlueHist;
GdkPixbuf	*pixbufRedHist;
GdkPixbuf	*pixbufGreenHist;
GdkPixbuf	*pixbufBlueHist;
IplImage	*imgRed;
IplImage	*imgGreen;
IplImage	*imgBlue;
IplImage	*imgHistRed;
IplImage	*imgHistGreen;
IplImage	*imgHistBlue; 
CvHistogram 	*hist;
int 		numBins;
float 		range[2];
float 		**ranges;



GtkAdjustment	*adjHistLow;
GtkAdjustment	*adjHistHigh;
GtkEntry	*txtMinHist;
GtkEntry	*txtMaxHist;

void on_window_destroy(GtkObject*, gpointer); 
void UpdateButton(GtkObject*, gpointer);
void UpdateSliderRange(GtkObject *, gpointer);
GdkPixbuf *ConvertOpenCv2Pixbuf(IplImage*);

void GuiInit(char*);
char UpdateMainImage(IplImage*);
char UpdateRedHistImage(IplImage *);
char UpdateGreenHistImage(IplImage *);
char UpdateBlueHistImage(IplImage *);
 
#endif 
