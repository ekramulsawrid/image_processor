#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoStretch:
//
// Apply histogram stretching to I1. Output is in I2.
// Stretch intensity values between t1 and t2 to fill the range [0,255].
//
void
HW_histoStretch(ImagePtr I1, int t1, int t2, ImagePtr I2)
{
	// copy image header (width, height, and total number of pixels
	IP_copyImageHeader(I1, I2);

	//init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	//init lookup table
	int i, lut[MXGRAY];
	int range;
	if (t1 < t2) range = t2 - t1;
	else range = 1;
	for (i = 0; i < MXGRAY; ++i) {
		lut[i] = CLIP(int((255 * (i - t1)) / range), 0, MaxGray);
	}


	//declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

	//visit all image channels and evaluate output image
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch
		for (i = 0; i < total; i++) *p2++ = lut[*p1++];	// use lut[] to eval output
	}
}
