#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_convolve:
//
// Convolve I1 with filter kernel in Ikernel.
// Output is in I2.
//

void copyRowToCircBuffer(ChannelPtr<uchar>, short*, int, int);

void
HW_convolve(ImagePtr I1, ImagePtr Ikernel, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	int kSize = Ikernel->width();

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;
	ChannelPtr<float> pK;

	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);
		IP_getChannel(Ikernel, 0, pK, type);

		int val = kSize + h - 1;

		// PADDED IMAGE

		// declarations
		short** image = new short*[kSize + h - 1];
		for (int i = 0; i < kSize + h - 1; i++) {
			image[i] = new short[kSize + w - 1];
		}

		for (int i = 0; i < kSize / 2; i++) {					// pad
			copyRowToCircBuffer(p1, image[i], w, kSize);
		}

		for (int i = kSize / 2; i < h + kSize / 2; i++) {			// all rows
			copyRowToCircBuffer(p1, image[i], w, kSize);
			p1 = p1 + w;
		}

		for (int i = h + kSize / 2; i < h + kSize - 1; i++) {			// pad
			copyRowToCircBuffer(p1, image[i], w, kSize);
		}

		p1 = p1 - total;				// back to start

		// Do operation on for each (x, y) in rows, columns
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {

				float sum = 0.0;

				for (int i = 0 - kSize / 2; i < kSize / 2 + 1; i++) {
					for (int j = 0 - kSize / 2; j < kSize / 2 + 1; j++) {
						sum += image[y + kSize / 2 + i][x + kSize / 2 + j] * *pK++;
					}
				}

				pK -= kSize * kSize;
				p2[y * w + x] = (int)CLIP(sum, 0, 255);
			}
		}

		// delete padded image
		for (int i = 0; i < kSize + h - 1; i++) {
			delete[] image[i];
		}

		delete[] image;
	}


}


