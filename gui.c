#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <argp.h>

#include <cv.h>
#include <highgui.h>

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "pixelLib.h"
#include "starAlign.h"
#include "starStack.h"
#include "gui.h"

 
extern IplImage *imgStack;

void on_window_destroy (GtkObject *object, gpointer user_data) {
    gtk_main_quit();
}

void UpdateButton(GtkObject *object, gpointer user_data) {

	int 		numBins = 256;
	float 		range[] = {0, 255};
	float 		*ranges[] = { range };


	cvReleaseImage(&imgStack8bit);
	imgStack8bit = AdjustHistogram(imgStack, (float)gtk_adjustment_get_value(adjHistLow), (float)gtk_adjustment_get_value(adjHistHigh));
	UpdateMainImage(imgStack8bit);	

	//Generate histogram from 8bit stacked image
	hist = cvCreateHist(1, &numBins, CV_HIST_ARRAY, ranges, 1);
	cvClearHist(hist);
	
	imgRed = cvCreateImage(cvGetSize(imgStack8bit), 8, 1);
	imgGreen = cvCreateImage(cvGetSize(imgStack8bit), 8, 1);
	imgBlue = cvCreateImage(cvGetSize(imgStack8bit), 8, 1);

	cvSplit(imgStack8bit, imgBlue, imgGreen, imgRed, NULL);

	cvCalcHist(&imgRed, hist, 0, 0);
	imgHistRed = DrawHistogram(hist,1,1);
	cvClearHist(hist);

	cvCalcHist(&imgGreen, hist, 0, 0);
	imgHistGreen = DrawHistogram(hist,1,1);
	cvClearHist(hist);
 
	cvCalcHist(&imgBlue, hist, 0, 0);
	imgHistBlue = DrawHistogram(hist,1,1);
 	cvClearHist(hist);

	//Display the histogram images
	UpdateRedHistImage(imgHistRed);
	UpdateGreenHistImage(imgHistGreen);	 
	UpdateBlueHistImage(imgHistBlue);	 


}

void UpdateSliderRange(GtkObject *object, gpointer user_data) {
	double tempMinHist, tempMaxHist;
	char	entryText[10];
	tempMinHist = atof(gtk_entry_get_text(txtMinHist));
	tempMaxHist = atof(gtk_entry_get_text(txtMaxHist));

	if(tempMinHist > tempMaxHist) {
		tempMaxHist = tempMinHist + 1;
		sprintf(entryText,"%.3f",tempMaxHist);
		gtk_entry_set_text(txtMaxHist, entryText);
	}
	if(tempMaxHist < tempMinHist) {
		tempMinHist = tempMaxHist - 1;
		sprintf(entryText,"%.3f",tempMinHist);
		gtk_entry_set_text(txtMinHist, entryText);
	}
	//Make sure the sliders do not go out of range
	if(tempMinHist > gtk_adjustment_get_value(adjHistLow)) {
		gtk_adjustment_set_value(adjHistLow, tempMinHist);
	}
	if(tempMaxHist < gtk_adjustment_get_value(adjHistLow)) {
		gtk_adjustment_set_value(adjHistLow, tempMaxHist);
	}
	
	//Make sure the sliders do not go out of range
	if(tempMinHist > gtk_adjustment_get_value(adjHistHigh)) {
		gtk_adjustment_set_value(adjHistHigh, tempMinHist);
	}
	if(tempMaxHist < gtk_adjustment_get_value(adjHistHigh)) {
		gtk_adjustment_set_value(adjHistHigh, tempMaxHist);
	}
	
	gtk_adjustment_set_lower(adjHistLow, tempMinHist);
	gtk_adjustment_set_upper(adjHistLow, tempMaxHist);

	gtk_adjustment_set_lower(adjHistHigh, tempMinHist);
	gtk_adjustment_set_upper(adjHistHigh, tempMaxHist);

	
}

GdkPixbuf *ConvertOpenCv2Pixbuf(IplImage *image){ 
	gtkMask = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, image->nChannels);
	// Usually opencv image is BGR, so we need to change it to RGB
	cvCvtColor( image, gtkMask, CV_BGR2RGB ); 
	GdkPixbuf *pix; 
	pix = gdk_pixbuf_new_from_data ((guchar*)gtkMask->imageData, 
					GDK_COLORSPACE_RGB, 
					FALSE, 
					gtkMask->depth, 
					gtkMask->width, 
					gtkMask->height, 
					(gtkMask->widthStep), 
					NULL, 
					NULL); 
	return pix; 
}

void GuiInit(char *guiFilename) {
	GtkBuilder      *builder; 
	GtkWidget       *window;
   	GtkLabel	*labelStatus;

	int 		numBins = 256;
	float 		range[] = {0, 255};
	float 		*ranges[] = { range };


    	builder = gtk_builder_new();
    	gtk_builder_add_from_file(builder, guiFilename, NULL);
	//Assign all the widgets
    	window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    	imageMain = GTK_WIDGET(gtk_builder_get_object(builder, "imageMain"));
    	
    	imageRedHist = GTK_WIDGET(gtk_builder_get_object(builder, "imageRedHist"));
    	imageGreenHist = GTK_WIDGET(gtk_builder_get_object(builder, "imageGreenHist"));
    	imageBlueHist = GTK_WIDGET(gtk_builder_get_object(builder, "imageBlueHist"));

	labelStatus = GTK_WIDGET(gtk_builder_get_object(builder, "labelStatus"));
    	adjHistLow = GTK_WIDGET(gtk_builder_get_object(builder, "adjHistLow"));
    	adjHistHigh = GTK_WIDGET(gtk_builder_get_object(builder, "adjHistHigh"));
	txtMinHist = GTK_WIDGET(gtk_builder_get_object(builder, "txtMinHist"));
	txtMaxHist = GTK_WIDGET(gtk_builder_get_object(builder, "txtMaxHist"));
	//Assign all the function calls
	gtk_builder_connect_signals(builder, NULL);
	//unreference the builder object
    	g_object_unref(G_OBJECT(builder));

	gtk_widget_show(window);               

	gtk_label_set_text(labelStatus, ""); 
	//convert stacked image to 8bit image
	imgStack8bit = AdjustHistogram(imgStack, (float)0.0, (float)255.0);

	//Generate histogram from 8bit stacked image
	
	hist = cvCreateHist(1, &numBins, CV_HIST_ARRAY, ranges, 1);
	cvClearHist(hist);
	
	imgRed = cvCreateImage(cvGetSize(imgStack8bit), 8, 1);
	imgGreen = cvCreateImage(cvGetSize(imgStack8bit), 8, 1);
	imgBlue = cvCreateImage(cvGetSize(imgStack8bit), 8, 1);

	cvSplit(imgStack8bit, imgBlue, imgGreen, imgRed, NULL);

	cvCalcHist(&imgRed, hist, 0, 0);
	imgHistRed = DrawHistogram(hist,1,1);
	cvClearHist(hist);

	cvCalcHist(&imgGreen, hist, 0, 0);
	imgHistGreen = DrawHistogram(hist,1,1);
	cvClearHist(hist);
 
	cvCalcHist(&imgBlue, hist, 0, 0);
	imgHistBlue = DrawHistogram(hist,1,1);
 	cvClearHist(hist);

	//Display the histogram images
	UpdateRedHistImage(imgHistRed);
	UpdateGreenHistImage(imgHistGreen);	 
	UpdateBlueHistImage(imgHistBlue);	 


	//Update the main image with the scaled stack image
	UpdateMainImage(imgStack8bit);	 

		 
	//Enter the GUI main
	gtk_main();
}

char UpdateMainImage(IplImage *image) {
	pixbufMain = ConvertOpenCv2Pixbuf(image);
	pixbufMain = gdk_pixbuf_scale_simple(pixbufMain, GTK_WIDGET(imageMain)->allocation.width, GTK_WIDGET(imageMain)->allocation.height, GDK_INTERP_BILINEAR);
	gtk_image_set_from_pixbuf(imageMain, pixbufMain);
	return 1;
}

char UpdateRedHistImage(IplImage *image) {
	pixbufRedHist = ConvertOpenCv2Pixbuf(image);
	pixbufRedHist = gdk_pixbuf_scale_simple(pixbufRedHist, GTK_WIDGET(imageRedHist)->allocation.width, GTK_WIDGET(imageRedHist)->allocation.height, GDK_INTERP_BILINEAR);
	gtk_image_set_from_pixbuf(imageRedHist, pixbufRedHist);
	return 1;
}

char UpdateGreenHistImage(IplImage *image) {
	pixbufGreenHist = ConvertOpenCv2Pixbuf(image);
	pixbufGreenHist = gdk_pixbuf_scale_simple(pixbufGreenHist, GTK_WIDGET(imageGreenHist)->allocation.width, GTK_WIDGET(imageGreenHist)->allocation.height, GDK_INTERP_BILINEAR);
	gtk_image_set_from_pixbuf(imageGreenHist, pixbufGreenHist);
	return 1;
}

char UpdateBlueHistImage(IplImage *image) {
	pixbufBlueHist = ConvertOpenCv2Pixbuf(image);
	pixbufBlueHist = gdk_pixbuf_scale_simple(pixbufBlueHist, GTK_WIDGET(imageBlueHist)->allocation.width, GTK_WIDGET(imageBlueHist)->allocation.height, GDK_INTERP_BILINEAR);
	gtk_image_set_from_pixbuf(imageBlueHist, pixbufBlueHist);
	return 1;
}
