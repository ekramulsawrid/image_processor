#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_blur:
//
// Blur image I1 with a box filter (unweighted averaging).
// The filter has width filterW and height filterH.
// We force the kernel dimensions to be odd.
// Output is in I2.
//
void
HW_blur(ImagePtr I1, int filterW, int filterH, ImagePtr I2)
{

	IP_copyImageHeader(I1, I2);		// copy image header (width, height) of input image I1 to output image I2
	ImagePtr I3;					// declaration of image pointer
	IP_copyImageHeader(I1, I3);		// copy image header (width, height) of input image I1 to output image I3

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I2->height();
	int total = w * h;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2, p3;
	int type;

	// set to odd
	if (filterW % 2 == 0) {
		filterW = filterW + 1;
	}
	if (filterH % 2 == 0) {
		filterH = filterH + 1;
	}

	// visit all image channels and evaluate output image
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);							// get output pointer for channel ch
		IP_getChannel(I3, ch, p3, type);							// get output pointer for channel ch
		for (int y = 0; y < h; y++) {					// first pass (blur rows)
			short* buffer = new short[w + filterW - 1];				// declaration of buffer

			for (int i = 0; i < filterW / 2; i++) {			// pad
				buffer[i] = *p1;
			}
			int pos = 0;
			for (int i = filterW / 2; i < w + filterW / 2; i++) {		// row
				buffer[i] = p1[pos];
				pos += 1;
			}
			for (int i = w + filterW / 2; i < w + filterW - 1; i++) {		// pad
				buffer[i] = p1[pos - 1];
			}
			short sum = 0;
			for (int i = 0; i < filterW; i++) {				// get first sum
				sum += buffer[i];
			}
			for (int i = 0; i < w; i++) {				// visit every input columns
				p3[i] = sum / filterW;						// average
				sum += buffer[i + filterW] - buffer[i];			// update sum
			}

			delete[] buffer;

			p1 = p1 + w;
			p3 = p3 + w;
		}

		p3 = p3 - total;		// back to start

		for (int x = 0; x < w; x++) {					// second pass (blur columns)
			short* buffer = new short[h + filterH - 1];

			for (int i = 0; i < filterH / 2; i++) {			// pad
				buffer[i] = *p3;
			}
			int pos = 0;
			for (int i = filterH / 2; i < h + filterH / 2; i++) {		// row
				buffer[i] = p3[pos];
				pos += w;
			}
			for (int i = h + filterH / 2; i < h + filterH - 1; i++) {		// pad
				buffer[i] = p3[pos - w];
			}

			short sum = 0;
			for (int i = 0; i < filterH; i++) {				// get first sum
				sum += buffer[i];
			}
			for (int i = 0; i < h; i++) {				// visit every input rows
				p2[i * w] = sum / filterH;					// average
				sum += buffer[i + filterH] - buffer[i];			// update sum
			}

			delete[] buffer;

			p2 = p2 + 1;
			p3 = p3 + 1;
		}
	}
}
