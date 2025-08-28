#include <stdio.h>
#include <sys/types.h>
#include <limits.h>
#include <glib.h>
//#include "reftek.h"
#include "steim.h"

enum {
	LITTLE_END,
	BIG_END
};

#ifdef WIN32
void revmem(u_char *buf, int size)
{
	u_char tmp[4];
	rmemcpy(tmp, buf, size);
	memcpy((char *)buf, (char *)tmp, size);
}

void rmemcpy(u_char *buf, u_char *rbuf, int size)
{
	register u_char *baddr, *raddr, *bufend, *rbufstart;

	bufend = buf+size;
	rbufstart = rbuf+size-1;

	for (baddr=buf, raddr=rbufstart; baddr<bufend; baddr++, raddr--)
		*baddr = *raddr;
}
#endif

int endian (void)
{
        int i, j, num;
        char * p;

        struct {
                short s;
                short s1;
        } d;

        /*  0000,0010,0000,0001   */
        d.s = 513;
        /*  0000,0100,0000,0011   */
        d.s1 = 1027;

        num = 0;
        j = 1;
        p= (char *) &d;
        for(i = sizeof (d) - 1; i >= 0; i--) {
                num +=  *(p + i) * j;
                j *= 10;
        }

        if (num == 1234) {
                /*   Little endian   */
                return (LITTLE_END);
        } else if (num == 4321 || num == 2143) {
                /*   Big endian   */
                return (BIG_END);
        }
        /*   Error   */
        return (-1);
}
static void unpack(register u_int w0, register u_int *code)
{
	register u_int *addr;
	register u_int mask = 0x00000003;

	for (addr=&code[15]; addr>=code; addr--) {
		*addr = w0 & mask;
		w0 >>= 2;
	}
}

static int one_byte (register int word, register int index)
{
	register int diff, shift;

	/* left shift to clobber junk on left		*/
	shift = index << 3;
	diff = word << shift;

	/* right shift convert to long and extend sign	*/
	diff >>= 24;

	return(diff);
}

static int two_byte (int word, int index)
{
	int diff, shift;

	/* left shift to clobber junk on left		*/
	shift = index << 4;
	diff = word << shift;

	/* right shift convert to long and extend sign	*/
	diff >>= 16;

	return(diff);
}

static int do_frame32 (Frame *frame, int *data, int index, int *xlast, gboolean skip_first, gboolean rev)
{
	u_int code[16];
	int i, j, k;
	int diff;
	int xnot;
	int w0;
	int sample;

	xnot = *xlast;

	if (rev)
		rmemcpy((u_char *)&w0, (u_char *)&frame->word[0], 4);
  	else
		memcpy(&w0, &frame->word[0], 4);

	unpack(w0, code);

	for (i=1, k=4; i<16; i++,k+=4) {

		switch (code[i]) {

			case 0:
				break;

			case 1:
			  /* 4 1-byte differences	*/
			  if (rev)
			    revmem((u_char*)&frame->word[i], 4);

				for (j=0; j<4; j++) 
				{
					diff = one_byte(frame->word[i], j);
					if(!skip_first || index != 0) {
					    sample = xnot + diff;
					} else {
					    sample = xnot;
					    //skipping first item in decompression					    
					}
					data[index++] = sample;
					xnot = sample;
				}
				break;

			case 2:
				/* 2 2-byte differences	*/
			  if (rev)
			    revmem((u_char*)&frame->word[i], 4);

				for (j=0; j<2; j++) 
				{
					diff = two_byte(frame->word[i], j);
					if(!skip_first || index != 0) {
					    sample = xnot + diff;
					} else {
					    sample = xnot;
					    //skipping first item in decompression					    
					}
					data[index++] = sample;
					xnot = sample;
				}
				break;

			case 3:
				/* 1 4-byte difference	*/
				if (rev)
				  rmemcpy((u_char *)&diff, (u_char *) &frame->word[i], 4);
			  	else
					memcpy(&diff, &frame->word[i], 4);

				if(!skip_first || index != 0) {
				    sample = xnot + diff;
				} else {
				    sample = xnot;
				    //skipping first item in decompression
				}
				data[index++] = sample;
				xnot = sample;
				break;

			default:
				/* error		*/
				fprintf(stderr, "Illegal control code.\n");
				return (-1);
		}

	}
	*xlast = xnot;
	return(index);
}

int steim1 (char *block, int *data, int *last_x, int blocksize, gboolean rev)
{
	int index;
	int x0, xn;
	int i;
	gboolean skip_first = FALSE;

	/* Get SEED integration constants.	*/
    if (rev) {
	  rmemcpy((u_char *)&x0, (u_char *)&block[68], 4);
	  rmemcpy((u_char *)&xn, (u_char *)&block[72], 4);
	}
	else
	{
		memcpy(&x0, &block[68], 4);
		memcpy(&xn, &block[72], 4);
	}
	if (*last_x == INT_MIN) {
	  *last_x = x0;
	  skip_first = TRUE;
	}

	for (i=64, index = 0; i<blocksize; i+=64)
		index = do_frame32((Frame *) &block[i], data, index, last_x, skip_first, rev);

	if (index == -1)
		return(-1);

	if (index == 0)
		return(index);

	// Store Xn to compute X0 in next block
	*last_x = xn;

#ifdef DEBUG
	if ((x0 != data[0]) || (xn != data[index-1])) {
		fprintf (stderr, "ERROR in Steim decompression\n");
	}
#endif

	return(index);
}

#if false
static int do_frame16 (Frame *frame, short *data, int index, int *xlast)
{
	u_int code[16];
	int i, j, k;
	int diff;
	int xnot;
	int w0;
	int sample;

	xnot = *xlast;

	memcpy(&w0, &frame->word[0], 4);
	if (endian () == LITTLE_END) 
	  rmemcpy((u_char *)&w0, (u_char *)&frame->word[0], 4);

	unpack(w0, code);

	for (i=1, k=4; i<16; i++,k+=4) {

		switch (code[i]) {

			case 0:
				break;

			case 1:
			  /* 4 1-byte differences	*/
			  if (endian () == LITTLE_END) {
			    for (j=3; j>-1; j--) {
			      diff = one_byte(frame->word[i], j);
			      sample = xnot + diff;
			      data[index++] = (short) sample;
			      xnot = sample;
			    }
			  } else {
			    for (j=0; j<4; j++) {
			      diff = one_byte(frame->word[i], j);
			      sample = xnot + diff;
			      data[index++] = (short) sample;
			      xnot = sample;
			    }
			  }
			  break;
			  
			case 2:
			  /* 2 2-byte differences	*/
			  if (endian () == LITTLE_END) {
			    for (j=1; j>-1; j--) {
			      diff = two_byte(frame->word[i], j);
			      sample = xnot + diff;
			      data[index++] = (short) sample;
			      xnot = sample;
			    }
			  } else {
			    for (j=0; j<2; j++) {
			      diff = two_byte(frame->word[i], j);
			      sample = xnot + diff;
			      data[index++] = (short) sample;
			      xnot = sample;
			    }
			  }
			  break;

			case 3:
				/* 1 4-byte difference	*/
				memcpy(&diff, &frame->word[i], 4);
				if (endian () == LITTLE_END)
				  rmemcpy((u_char *)&diff, (u_char *)&frame->word[i], 4);

				sample = xnot + diff;
				data[index++] = (short) sample;
				xnot = sample;
				break;

			default:
				/* error		*/
				fprintf(stderr, "Illegal control code.\n");
				return(-1);
		}

	}
	*xlast = xnot;
	return(index);
}

int uncompress16 (char *block, short *data, int *last_x, int blocksize)
{
	int index=0;
	int x0, xn;
	int i;

	/* Need to do xfile header here.	*/

	/* Get SEED integration constants.	*/
	memcpy(&x0, &block[68], 4);
	memcpy(&xn, &block[72], 4);
	if (endian () == LITTLE_END) {
	  rmemcpy((u_char *)&x0, (u_char *)&block[68], 4);
	  rmemcpy((u_char *)&xn, (u_char *)&block[72], 4);
	}

	for (i=64; i<blocksize; i+=64)
		index = do_frame16((Frame *) &block[i], data, index, last_x);

	if (index == -1)
		return(-1);
		
	if (index == 0)
		return(index);

	/* Store Xn to compute X0 in next block	*/
	*last_x = xn;

#ifdef DEBUG
	if ((x0 != data[0]) || (xn != data[index-1]))
		fprintf (stderr, "ERROR in Steim decompression\n");
#endif

	return(index);
}
#endif
