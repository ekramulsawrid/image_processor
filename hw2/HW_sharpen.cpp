#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_sharpen:
//
// Sharpen image I1. Output is in I2.
//

void HW_blur(ImagePtr, int, int, ImagePtr);

void
HW_sharpen(ImagePtr I1, int size, double factor, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

	HW_blur(I1, size, size, I2);	// output blurred image to I2

	// visit all image channels and evaluate output image
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {		// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);			// get output pointer for channel ch
		for (int i = 0; i < total; i++) {			// visit every pixel
			*p2 = CLIP((*p1 - *p2) * factor + *p1, 0, MaxGray);		// use formula
			*p1++;
			*p2++;
		}
	}
}
