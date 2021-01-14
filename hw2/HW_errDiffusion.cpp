#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_errDiffusion:
//
// Apply error diffusion algorithm to image I1.
//
// This procedure produces a black-and-white dithered version of I1.
// Each pixel is visited and if it + any error that has been diffused to it
// is greater than the threshold, the output pixel is white, otherwise it is black.
// The difference between this new value of the pixel from what it used to be
// (somewhere in between black and white) is diffused to the surrounding pixel
// intensities using different weighting systems.
//
// Use Floyd-Steinberg     weights if method=0.
// Use Jarvis-Judice-Ninke weights if method=1.
//
// Use raster scan (left-to-right) if serpentine=0.
// Use serpentine order (alternating left-to-right and right-to-left) if serpentine=1.
// Serpentine scan prevents errors from always being diffused in the same direction.
//
// A circular buffer is used to pad the edges of the image.
// Since a pixel + its error can exceed the 255 limit of uchar, shorts are used.
//
// Apply gamma correction to I1 prior to error diffusion.
// Output is saved in I2.
//

void gammaCorrection(ImagePtr, double, ImagePtr);
void copyRowToCircBuffer(ChannelPtr<uchar>, short*, int, int);

void
HW_errDiffusion(ImagePtr I1, int method, bool serpentine, double gamma, ImagePtr I2)
{

	IP_copyImageHeader(I1, I2);		// copy image header (width, height) of input image I1 to output image I2
	ImagePtr I3;
	IP_copyImageHeader(I1, I3);		// copy image header (width, height) of input image I1 to output image I3
	gammaCorrection(I1, gamma, I3);			// apply gamma correction

	// init vars for width, height, total number of pixels, and threshold
	int w = I1->width();
	int h = I1->height();
	int total = w * h;
	int thr = MXGRAY / 2;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

	if (method == 0) {		// Floyd-Steinberg Algorithm

		// declarations for pointers and error
		short* in1;
		short* in2;
		short e;

		// buffers
		short* buffer1 = new short[w + 2];
		short* buffer2 = new short[w + 2];

		if (serpentine == 1) {				// serpentine scan

			// visit all image channels and evaluate output image
			for (int ch = 0; IP_getChannel(I3, ch, p1, type); ch++) {		// get input  pointer for channel ch
				IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch

				copyRowToCircBuffer(p1, buffer1, w, 3);		// copy first row to circular buffer
				p1 = p1 + w;

				for (int y = 1; y < h; y++) {				// visit all input rows
					if (y % 2 == 0) {						// even row
						copyRowToCircBuffer(p1, buffer1, w, 3);		// copy row to buffer
						p1 = p1 + w;
						in1 = buffer2 + w + 1;
						in2 = buffer1 + w + 1;

						p2 = p2 + w - 1;							// advance pointer
						for (int x = 0; x < w; x++) {				// visit all input columns
							*p2 = (*in1 < thr) ? 0 : 255;			// threshold
							e = *in1 - *p2;							// evaluate error

							*(in1 - 1) += (e * 7 / 16.0);			// apply error
							*(in2 - 1) += (e * 3 / 16.0);			// apply error
							*(in2) += (e * 5 / 16.0);				// apply error
							*(in2 - 1) += (e * 1 / 16.0);			// apply error

							in1--;
							in2--;
							p2--;
						}

						p2 = p2 + w + 1;

					}
					else {									// odd row
						copyRowToCircBuffer(p1, buffer2, w, 3);			// copy row to circular buffer
						p1 = p1 + w;
						in1 = buffer1 + 1;
						in2 = buffer2 + 1;

						for (int x = 0; x < w; x++) {				// visit all input columns
							*p2 = (*in1 < thr) ? 0 : 255;			// threshold
							e = *in1 - *p2;							// evaluate error

							*(in1 + 1) += (e * 7 / 16.0);			// apply error
							*(in2 - 1) += (e * 3 / 16.0);			// apply error
							*(in2) += (e * 5 / 16.0);				// apply error
							*(in2 + 1) += (e * 1 / 16.0);			// apply error

							in1++;
							in2++;
							p2++;
						}
					}
				}
			}
		}
		else {								// raster scan
			// visit all image channels and evaluate output image
			for (int ch = 0; IP_getChannel(I3, ch, p1, type); ch++) {		// get input  pointer for channel ch
				IP_getChannel(I2, ch, p2, type);							// get input  pointer for channel ch

				copyRowToCircBuffer(p1, buffer1, w, 3);						// copy first row to circular buffer
				p1 = p1 + w;

				for (int y = 1; y < h; y++) {								// visit all input rows	
					if (y % 2 == 0) {										// even row
						copyRowToCircBuffer(p1, buffer1, w, 3);				// copy row to circular buffer
						in1 = buffer2 + 1;
						in2 = buffer1 + 1;
					}
					else {													// odd row
						copyRowToCircBuffer(p1, buffer2, w, 3);				// copy row to circular buffer
						in1 = buffer1 + 1;
						in2 = buffer2 + 1;
					}

					p1 = p1 + w;

					for (int x = 0; x < w; x++) {							// visit all input columns
						*p2 = (*in1 < thr) ? 0 : 255;						// threshold
						e = *in1 - *p2;										// evaluate error

						*(in1 + 1) += (e * 7 / 16.0);						// apply error
						*(in2 - 1) += (e * 3 / 16.0);						// apply error
						*(in2) += (e * 5 / 16.0);							// apply error
						*(in2 + 1) += (e * 1 / 16.0);						// apply error

						in1++;
						in2++;
						p2++;
					}
				}
			}
		}
		// delete buffers 
		delete[] buffer1;
		delete[] buffer2;
	}
	else {		// Jarvis-Judice-Ninke Algorithm

		int e;
		int type;
		short** in = new short*[3];
		short** buffer = new short*[3];

		for (int i = 0; i < 3; i++) {
			buffer[i] = new short[w + 4];
		}

		if (serpentine == 1) {

			for (int ch = 0; IP_getChannel(I3, ch, p1, type); ch++) {
				IP_getChannel(I2, ch, p2, type);

				copyRowToCircBuffer(p1, buffer[0], w, 5);
				copyRowToCircBuffer(p1, buffer[1], w, 5);

				p1 += w;
				for (int y = 2; y < h; y++) {						// visit every input row

					if (y % 3 == 2) {
						copyRowToCircBuffer(p1, buffer[2], w, 5);
					}
					else if (y % 3 == 1) {
						copyRowToCircBuffer(p1, buffer[1], w, 5);
					}
					else if (y % 3 == 0) {
						copyRowToCircBuffer(p1, buffer[0], w, 5);
					}
					p1 += w;

					if (y % 2 == 0) {					// even row
						if (y % 3 == 0) {
							in[0] = buffer[1] + w + 2;
							in[1] = buffer[2] + w + 2;
							in[2] = buffer[0] + w + 2;
						}
						else if (y % 3 == 1) {
							in[0] = buffer[2] + w + 2;
							in[1] = buffer[0] + w + 2;
							in[2] = buffer[1] + w + 2;
						}
						else {
							in[0] = buffer[0] + w + 2;
							in[1] = buffer[1] + w + 2;
							in[2] = buffer[2] + w + 2;
						}

						p2 += w - 1;
						for (int i = 0; i < w; i++) {					// visit every input column
							*p2 = (*in[0] < thr) ? 0 : 255;
							e = *in[0] - *p2;

							*(in[0] - 1) += (e * 7 / 48.0);
							*(in[0] - 2) += (e * 5 / 48.0);
							*(in[1]) += (e * 7 / 48.0);
							*(in[1] + 1) += (e * 5 / 48.0);
							*(in[1] + 2) += (e * 3 / 48.0);
							*(in[1] - 1) += (e * 5 / 48.0);
							*(in[1] - 2) += (e * 3 / 48.0);
							*(in[2]) += (e * 5 / 48.0);
							*(in[2] + 1) += (e * 3 / 48.0);
							*(in[2] + 2) += (e * 1 / 48.0);
							*(in[2] - 1) += (e * 3 / 48.0);
							*(in[2] - 2) += (e * 1 / 48.0);

							in[0]--;
							in[1]--;
							in[2]--;
							p2--;
						}
						p2 += w + 1;

					}
					else {								// odd row
						if (y % 3 == 1) {
							in[0] = buffer[2] + 2;
							in[1] = buffer[0] + 2;
							in[2] = buffer[1] + 2;
						}
						else if (y % 3 == 0) {
							in[0] = buffer[1] + 2;
							in[1] = buffer[2] + 2;
							in[2] = buffer[0] + 2;
						}
						else {
							in[0] = buffer[0] + 2;
							in[1] = buffer[1] + 2;
							in[2] = buffer[2] + 2;
						}
						for (int i = 0; i < w; i++) {
							*p2 = (*in[0] < thr) ? 0 : 255;
							e = *in[0] - *p2;

							*(in[0] + 1) += (e * 7 / 48.0);
							*(in[0] + 2) += (e * 5 / 48.0);
							*(in[1]) += (e * 7 / 48.0);
							*(in[1] + 1) += (e * 5 / 48.0);
							*(in[1] + 2) += (e * 3 / 48.0);
							*(in[1] - 1) += (e * 5 / 48.0);
							*(in[1] - 2) += (e * 3 / 48.0);
							*(in[2]) += (e * 5 / 48.0);
							*(in[2] + 1) += (e * 3 / 48.0);
							*(in[2] + 2) += (e * 1 / 48.0);
							*(in[2] - 1) += (e * 3 / 48.0);
							*(in[2] - 2) += (e * 1 / 48.0);

							in[0]++;
							in[1]++;
							in[2]++;
							p2++;
						}
					}
				}
			}
		}
		else {							// raster scan

			for (int ch = 0; IP_getChannel(I3, ch, p1, type); ch++) {
				IP_getChannel(I2, ch, p2, type);

				copyRowToCircBuffer(p1, buffer[0], w, 5);
				copyRowToCircBuffer(p1, buffer[1], w, 5);

				p1 += w;

				for (int y = 2; y < h; y++) {					// visit every input row
					if (y % 3 == 2) {
						copyRowToCircBuffer(p1, buffer[2], w, 5);
					}
					else if (y % 3 == 1) {
						copyRowToCircBuffer(p1, buffer[1], w, 5);
					}
					else if (y % 3 == 0) {
						copyRowToCircBuffer(p1, buffer[0], w, 5);
					}
					p1 += w;

					if (y % 3 == 1) {
						in[0] = buffer[2] + 2;
						in[1] = buffer[0] + 2;
						in[2] = buffer[1] + 2;
					}
					else if (y % 3 == 0) {
						in[0] = buffer[1] + 2;
						in[1] = buffer[2] + 2;
						in[2] = buffer[0] + 2;
					}
					else {
						in[0] = buffer[0] + 2;
						in[1] = buffer[1] + 2;
						in[2] = buffer[2] + 2;
					}
					for (int i = 0; i < w; i++) {						// visit every input column
						*p2 = (*in[0] < thr) ? 0 : 255;
						e = *in[0] - *p2;
						*(in[0] + 1) += (e * 7 / 48.0);
						*(in[0] + 2) += (e * 5 / 48.0);
						*(in[1]) += (e * 7 / 48.0);
						*(in[1] + 1) += (e * 5 / 48.0);
						*(in[1] + 2) += (e * 3 / 48.0);
						*(in[1] - 1) += (e * 5 / 48.0);
						*(in[1] - 2) += (e * 3 / 48.0);
						*(in[2]) += (e * 5 / 48.0);
						*(in[2] + 1) += (e * 3 / 48.0);
						*(in[2] + 2) += (e * 1 / 48.0);
						*(in[2] - 1) += (e * 3 / 48.0);
						*(in[2] - 2) += (e * 1 / 48.0);

						in[0]++;
						in[1]++;
						in[2]++;
						p2++;
					}
				}
			}
		}
		// delete buffer
		delete[] buffer;
	}
}


void
gammaCorrection(ImagePtr I1, double gamma, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	// init lookup table
	int i, lut[MXGRAY];
	for (i = 0; i < MXGRAY; ++i) {
		lut[i] = (int)CLIP((MaxGray * (pow((double)i / MaxGray, 1.0 / gamma))), 0, MaxGray);
	}

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

	// visit all image channels and evaluate output image
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);		// get output pointer for channel ch
		for (i = 0; i < total; i++) *p2++ = lut[*p1++];	// use lut[] to eval output
	}
}


void copyRowToCircBuffer(ChannelPtr<uchar> p, short* buffer, int w, int size) {

	for (int i = 0; i < size / 2; i++) {			// padded area
		buffer[i] = *p;
	}
	for (int i = size / 2; i < size / 2 + w - 1; i++) {		// row
		buffer[i] = *p++;
	}
	for (int i = size / 2 + w - 1; i < size + w - 1; i++) {		// padded area
		buffer[i] = *p;
	}
}
