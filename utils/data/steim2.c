#include <stdio.h>
#include <sys/types.h>
#include <limits.h>
#include <glib.h>
#include "steim.h"

/*** * Credit to Phillip Crotwell for the diffs extract routine...
 	 * Extracts differences from the next 64 byte frame of the given compressed
 	 * byte array (starting at offset) and returns those differences in an int
 	 * array.
 	 * An offset of 0 means that we are at the first frame, so include the header
 	 * bytes in the returned int array...else, do not include the header bytes
 	 * in the returned array.
 	 * @param bytes byte array of compressed data differences
 	 * @param offset index to begin reading compressed bytes for decoding
 	 * @param swapBytes reverse the endian-ness of the compressed bytes being read
 	 * @return integer array of difference (and constant) values
 	 */
static int extractDiffs(char *bytes, int *diffs, gboolean rev) 
{
	int nibbles;
	int currNibble = 0;
	int dnib = 0;
	int tempInt, i, d;
	int currNum = 0, offset = 0;

	if (rev)
		rmemcpy((u_char *)&nibbles, (u_char *) &bytes[offset], 4);
	else
		memcpy(&nibbles, &bytes[offset], 4);

	for (i=1; i<16; i++) 
	{
		currNibble = (nibbles >> (30 - i*2 ) ) & 0x03;
		switch (currNibble) {
			case 1:
				diffs[currNum++] = (int) (bytes[offset+(i*4)]);
				diffs[currNum++] = (int) (bytes[offset+(i*4)+1]);
				diffs[currNum++] = (int) (bytes[offset+(i*4)+2]);
				diffs[currNum++] = (int) (bytes[offset+(i*4)+3]);
				break;
			case 2:
				if (rev)
					rmemcpy((u_char *)&tempInt, (u_char *) &bytes[offset+(i*4)], 4);
				else
					memcpy(&tempInt, &bytes[offset+(i*4)], 4);
				dnib = (tempInt >> 30) & 0x03;
				switch (dnib) 
				{
					case 1:
						diffs[currNum++] = (tempInt << 2) >> 2;
						break;
					case 2:
						diffs[currNum++] = (tempInt << 2) >> 17;  // d0
						diffs[currNum++] = (tempInt << 17) >> 17; // d1
						break;
					case 3:
						diffs[currNum++] = (tempInt << 2) >> 22;  // d0
						diffs[currNum++] = (tempInt << 12) >> 22; // d1
						diffs[currNum++] = (tempInt << 22) >> 22; // d2
						break;
					default:
						break;
				}
				break;
			case 3:
				if (rev)
					rmemcpy((u_char *)&tempInt, (u_char *) &bytes[offset+(i*4)], 4);
				else
					memcpy(&tempInt, &bytes[offset+(i*4)], 4);
				dnib = (tempInt >> 30) & 0x03;
				// for case 3, we are going to use a for-loop formulation that
				// accomplishes the same thing as case 2, just less verbose.
				int diffCount = 0;  // number of differences
				int bitSize = 0;    // bit size
				int headerSize = 0; // number of header/unused bits at top
				switch (dnib) 
				{
					case 0:
						//System.out.println("3,0 means 5 six bit differences");
						headerSize = 2;
						diffCount = 5;
						bitSize = 6;
						break;
					case 1:
						//System.out.println("3,1 means 6 five bit differences");
						headerSize = 2;
						diffCount = 6;
						bitSize = 5;
						break;
					case 2:
						//System.out.println("3,2 means 7 four bit differences, with 2 unused bits");
						headerSize = 4;
						diffCount = 7;
						bitSize = 4;
						break;
					default:
						break;
				}
				if (diffCount > 0) {
					for (d=0; d<diffCount; d++) {  // for-loop formulation
						diffs[currNum++] = ( tempInt << (headerSize+(d*bitSize)) ) >> (((diffCount-1)*bitSize) + headerSize);
					}
				}
		}
	}
	return currNum;
}

#define TOTALDIFFS	(7*15+1)	// per frame = 7 samples per 15 long words + 1 nibble int

int steim2(char *b, int *samples, int *last_x, int blocksize, gboolean rev)
{
	int diffSamps[TOTALDIFFS];		
	int x0, xn;
	int current, j, i;
	int numSamps, totSamps;
	int lastValue=*last_x;
	gboolean first = (*last_x == INT_MIN ? TRUE : FALSE);

	// Get SEED integration constants
    if (rev) 
	{
		rmemcpy((u_char *)&x0, (u_char *)&b[68], 4);
		rmemcpy((u_char *)&xn, (u_char *)&b[72], 4);
	}
	else
	{
		memcpy(&x0, &b[68], 4);
		memcpy(&xn, &b[72], 4);
	}
	if (first)
		lastValue = *last_x = x0;

	for (i=64, totSamps = current = 0; i<blocksize; i+=64)
	{
		memset(diffSamps, 0, sizeof(int) * TOTALDIFFS);
		numSamps = extractDiffs((char *) &b[i], &diffSamps[0], rev);   // returns only differences
	
		// apply the differences to reconstruct our data points
		for (j = 0; j < numSamps; j++, current++)
		{
			lastValue = samples[current] = lastValue + diffSamps[j];  // X(n) = X(n-1) + d(n)
		}
		totSamps += numSamps;
	}

	*last_x = lastValue;
	return totSamps;
}
