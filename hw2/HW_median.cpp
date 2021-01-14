#include <algorithm>
#include <vector>
#include "IP.h"
using namespace IP;
using std::vector;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_median:
//
// Apply median filter of size sz x sz to I1.
// Clamp sz to 9.
// Output is in I2.
//

void copyRowToCircBuffer(ChannelPtr<uchar>, short*, int, int);

void
HW_median(ImagePtr I1, int sz, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	// make odd
	if (sz % 2 == 0) {
		sz = sz + 1;
	}

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

	short** buffer = new short*[sz];
	for (int i = 0; i < sz; i++) {				// buffers
		buffer[i] = new short[(sz + w - 1)];
	}

	int row = 0;			// keep track of buffer row

	// visit all image channels and evaluate output image
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {		// get output pointer for channel ch
		IP_getChannel(I2, ch, p2, type);			// get output pointer for channel ch

		for (int i = 0; i < sz / 2; i++) {				// pad
			copyRowToCircBuffer(p1, buffer[i], w, sz);
		}
		for (int i = sz / 2; i < sz; i++) {				// rows
			copyRowToCircBuffer(p1, buffer[i], w, sz);
			p1 = p1 + w;
		}

		std::vector<int> window(0);				// declaration for vector
		for (int y = 0; y < h; y++) {			// visit every input rows

			for (int i = 0; i < sz; i++) {				// fill vector with neighborhood pixel values
				for (int j = 0; j < sz; j++) {
					window.push_back(buffer[j][i]);
				}
			}

			for (int x = 0; x < w - 1; x++) {		// visit input rows

				// find middle value
				size_t middle = window.size() / 2;
				vector<int> sorted_window(0);
				sorted_window = window;
				std::nth_element(sorted_window.begin(), sorted_window.begin() + middle, sorted_window.end());

				*p2++ = sorted_window[middle];			// set pixel value

				// update window
				window.erase(window.begin(), window.begin() + sz);
				for (int i = 0; i < sz; i++) {
					window.push_back(buffer[i][x + sz]);
				}

			}
			for (int x = w - 1; x < w; x++) {		// rest of columns

				// find middle value
				size_t middle = window.size() / 2;
				vector<int> sorted_window(0);
				sorted_window = window;
				std::nth_element(sorted_window.begin(), sorted_window.begin() + middle, sorted_window.end());

				*p2++ = sorted_window[middle];			// set pixel value
			}

			window.clear();

			copyRowToCircBuffer(p1, buffer[row], w, sz);		// update a buffer for next row
			row = (row + 1) % sz;								// update to keep track of buffer row

			p1 = p1 + w;
		}

		// delete buffers
		for (int i = 0; i < sz; i++) {
			delete[] buffer[i];
		}

	}
}


