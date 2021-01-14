#include "IP.h"
using namespace IP;

void histoMatchApprox(ImagePtr, ImagePtr, ImagePtr);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoMatch:
//
// Apply histogram matching to I1. Output is in I2.
//
void
HW_histoMatch(ImagePtr I1, ImagePtr targetHisto, bool approxAlg, ImagePtr I2)
{
	if (approxAlg) {
		histoMatchApprox(I1, targetHisto, I2);
		return;
	}

	// copy image header (width, height) of input image I1 to output iamge I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	// declare vars
	int i, p, R;
	int left[MXGRAY], right[MXGRAY], reserve[MXGRAY], fullRight[MXGRAY];
	int Hsum, Havg, h1[MXGRAY];
	ChannelPtr<int> h2;
	double scale;

	// make input image histogram
	ChannelPtr<uchar> p1, p2;
	int type;
	for (i = 0; i < MXGRAY; i++) {
		h1[i] = 0;
		reserve[i] = 0;
		fullRight[i] = 0;
	}
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
		for (i = 0; i < total; i++) {
			h1[*p1]++;
			*p1++;
		}
	}

	// normalize h2 to conform with dimensions of I1
	Havg = 0;
	IP_getChannel(targetHisto, 0, h2, type);
	for (i = 0; i < MXGRAY; i++) {
		Havg += h2[i];
	}
	scale = (double)total / Havg;
	if (scale != 1.0) {
		for (i = 0; i < MXGRAY; i++) {
			h2[i] = round((double)h2[i] * scale);
		}
	}

	R = 0;
	Hsum = 0;
	// evaluate remapping of all input gray levels
	for (i = 0; i < MXGRAY; i++) {
		left[i] = R;	// left end of interval
		reserve[i] = h2[R] - Hsum;
		Hsum += h1[i];	// cumulative value for interval
		while (Hsum > h2[R] && R < MXGRAY - 1) {	// compute width of interval
			Hsum -= h2[R];	// adjust Hsum as interval widens
			R++;		// update
		}
		if (left[R] != R) fullRight[i] = left[i];
		right[i] = R;		// init right end of interval
	}

	// clear h1 and reuse it below
	for (i = 0; i < MXGRAY; i++) h1[i] = 0;

	// visit all input pixels
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);
		for (i = 0; i < total; i++) {
			p = left[*p1];
			if (h1[p] < h2[p]) {
				if (left[*p1] == fullRight[*p1]) {
					if (reserve[*p1] > 0) {
						*p2 = p;
						reserve[*p1] -= 1;
					}
					else {
						*p2 = p = left[*p1] = MIN(p + 1, right[*p1]);
					}
				}
				else *p2 = p;
			}
			else *p2 = p = left[*p1] = MIN(p + 1, right[*p1]);
			h1[p]++;
			*p1++;
			*p2++;
		}
	}

}

void
histoMatchApprox(ImagePtr I1, ImagePtr targetHisto, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output iamge I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	// declare vars
	int i, p, R;
	int left[MXGRAY], right[MXGRAY];
	int Hsum, Havg, h1[MXGRAY];
	ChannelPtr<int> h2;
	double scale;

	// make input image histogram
	ChannelPtr<uchar> p1, p2;
	int type;
	for (i = 0; i < MXGRAY; i++) h1[i] = 0;
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
		for (i = 0; i < total; i++) {
			h1[*p1]++;
			*p1++;
		}
	}

	// normalize h2 to conform with dimensions of I1
	Havg = 0;
	IP_getChannel(targetHisto, 0, h2, type);
	for (i = 0; i < MXGRAY; i++) {
		Havg += h2[i];
	}
	scale = (double)total / Havg;
	if (scale != 1.0) {
		for (i = 0; i < MXGRAY; i++) {
			h2[i] = ceil(h2[i] * scale);
		}
	}

	R = 0;
	Hsum = 0;
	// evaluate remapping of all input gray levels
	for (i = 0; i < MXGRAY; i++) {
		left[i] = R;	// left end of interval
		Hsum += h1[i];	// cumulative value for interval
		while (Hsum > h2[R] && R < MXGRAY - 1) {	// compute width of interval
			Hsum -= h2[R];	// adjust Hsum as interval widens
			R++;		// update
		}
		right[i] = R;		// init right end of interval
	}

	// clear h1 and reuse it below
	for (i = 0; i < MXGRAY; i++) h1[i] = 0;

	// visit all input pixels
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);
		for (i = 0; i < total; i++) {
			p = left[*p1];
			if (h1[p] < h2[p]) *p2 = p;		// mapping satisfies h2
			else *p2 = p = left[*p1] = MIN(p + 1, right[*p1]);
			h1[p]++;
			*p1++;
			*p2++;
		}
	}
}
