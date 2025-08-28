#include "nano2ah.h"
//#include "steim.h"
#include "pql_defines.h"
#include "pql_externs.h"

#include <sys/types.h>

int read_rechdr(char *, RECHDR *);
int	xdr_putdata(ahhed *, char *, XDR *);

typedef union seed_frame {
		char str[64];
		gint32 word[16];
} FRAME;

#ifdef ANSI_C
long do_block (char *, float [], long);
long do_frame (FRAME *, float [], long, long *);
static long one_byte (register long, register int);
long two_byte (long, int);
long rtwo_byte (long, int);
void rmemcpy();
#else
long do_block();
long do_frame();
static gint32 one_byte();
gint32 two_byte();
gint32 rtwo_byte();
void rmemcpy();
#endif

static void unpack();

long do_block(block, data, total)
char *block;
float data[];
long total;
{
	long index=0;
	long x0, xn;
	long xlast=0;
	long i;

	/* Need to do xfile header here.	*/

	/* Get SEED integration constants.	*/
#ifndef BIGENDIAN
	rmemcpy(&x0, &block[68], 4);
	rmemcpy(&xn, &block[72], 4);
#else
	memcpy(&x0, &block[68], 4);
	memcpy(&xn, &block[72], 4);
#endif
	xlast = x0;
	data[index++] = (float) x0;

	for (i=64; i<4096; i+=64) 
	{
		index = do_frame((FRAME *) &block[i], data, index, &xlast);
	}

	return(index);
}

long do_frame (frame, data, index, xlast)
FRAME *frame;
float data[];
long index;
long *xlast;
{
	u_int code[16];
	int i, j, k;
	long diff;
	long xnot;
	long w0;
	long sample;

	xnot = *xlast;

#ifndef BIGENDIAN
	rmemcpy(&w0, &frame->word[0], 4);
#else
	memcpy(&w0, &frame->word[0], 4);
#endif
	unpack(w0, code);

	for (i=1, k=4; i<16; i++,k+=4) {

		switch (code[i]) {

			case 0:
				break;

			case 1:
				/* 4 1-byte differences	*/
				for (j=0; j<4; j++) {
					diff = one_byte(frame->word[i], j);
					sample = xnot + diff;
					data[index++] = (float) sample;
					xnot = sample;
				}
				break;

			case 2:
				/* 2 2-byte differences	*/
				for (j=0; j<2; j++) {
#ifndef BIGENDIAN
					diff = rtwo_byte(frame->word[i], j);
#else
					diff = two_byte(frame->word[i], j);
#endif
					sample = xnot + diff;
					data[index++] = (float) sample;
					xnot = sample;
				}
				break;

			case 3:
				/* 1 4-byte difference	*/
#ifndef BIGENDIAN
				rmemcpy(&diff, &frame->word[i], 4);
#else
				memcpy(&diff, &frame->word[i], 4);
#endif
				sample = xnot + diff;
				data[index++] = (float) sample;
				xnot = sample;
				break;

			default:
				/* error		*/
				fprintf(stderr, "Illegal control code.\n");
				exit(1);
		}

	}
	*xlast = xnot;
	return(index);
}

static void unpack(w0, code)
register u_int w0;
register u_int code[];
{
	register u_int *addr;
	register u_int mask = 0x00000003;

	for (addr=&code[15]; addr>=code; addr--) {
		*addr = w0 & mask;
		w0 >>= 2;
	}
}

static gint32 one_byte (word, index)
register gint32 word;
register int index;
{
	register gint32 diff, shift;

	/* left shift to clobber junk on left		*/
	shift = index << 3;
	diff = word << shift;

	/* right shift convert to long and extend sign	*/
	diff >>= 24;

	return(diff);
}

#ifndef BIGENDIAN

gint32 rtwo_byte (word, index)
gint32 word;
int index;
{
	gint32 diff, shift, val;

	/* reverse byte order				*/
	rmemcpy(&val, &word, 4);


	/* left shift to clobber junk on left		*/
	shift = (1-index) << 4;
	diff = val << shift;

	/* right shift convert to long and extend sign	*/
	diff >>= 16;

	return(diff);
}

#else

gint32 two_byte (word, index)
gint32 word;
int index;
{
	gint32 diff, shift;

	/* left shift to clobber junk on left		*/
	shift = index << 4;
	diff = word << shift;

	/* right shift convert to long and extend sign	*/
	diff >>= 16;

	return(diff);
}

#endif
gint32 read_xdata(istream, xstream, ahhead)
FILE *istream;
XDR *xstream;
ahhed *ahhead;
{
	char block[4096];
	float data[3772];
	RECHDR rechdr;
	int num;
	static int blocksize=4096;
	gint32 total_samples = 0;
	gint32 samples_in_block;
	float max_amp_t();
	double epoch;

	num = fread(&block[0], sizeof(char), blocksize, istream);
	/* make sure you got a read			*/
	read_rechdr(block, &rechdr);

	epoch = (double) rechdr.sec;
	epoch += ((double) rechdr.msec) / 1000.0;
    passcal_etoh(&(ahhead->record.abstime), epoch);

	while (num == blocksize) 
	{
		samples_in_block = do_block(block, data, total_samples);
		total_samples += samples_in_block;

       	ahhead->record.ndata = samples_in_block;
       	xdr_putdata(ahhead, (char *) &data[0], xstream);

		ahhead->record.maxamp = max_amp_t (data, samples_in_block);
		num = fread(&block[0], sizeof(char), blocksize, istream);
	}

	return (total_samples);
}

gint32 read_xdata2(istream, car, ahhead)
FILE *istream;
char **car;
ahhed *ahhead;
{

  int      i;
  int      t_int;
  
  char     block[4096];
  float    data[3772];
  RECHDR   rechdr;
  int      num;
  static   int blocksize=4096;
  gint32     total_samples = 0;
  gint32     samples_in_block;
  float    max_amp_t();
  double   epoch;
  
  float   *f_ptr;

  num = fread(&block[0], sizeof(char), blocksize, istream);
  /* make sure you got a read			*/
  read_rechdr(block, &rechdr);
  
  epoch = (double) rechdr.sec;
  epoch += ((double) rechdr.msec) / 1000.0;
  passcal_etoh(&(ahhead->record.abstime), epoch);
  
  *car = NULL;
  
  
  while (num == blocksize) 
  {
    samples_in_block = do_block(block, data, total_samples);
    t_int = total_samples;
    total_samples += samples_in_block;

    if(*car == NULL) {
      *car = (char *)calloc(total_samples, sizeof(float));
      f_ptr = (float *)*car;
    }
    else {
      f_ptr = (float *)*car;
      *car  = (char *)realloc(f_ptr,total_samples*(sizeof(float)));
      f_ptr = (float *)( *car + t_int*(sizeof(float)));
    }
    
    for(i = 0; i < samples_in_block; i++, f_ptr++) 
	{
      *f_ptr = data[i];
    }
    
    ahhead->record.ndata = samples_in_block;
    // xdr_putdata(ahhead, (char *) &data[0], xstream);
    
    ahhead->record.maxamp = max_amp_t (data, samples_in_block);
    num = fread(&block[0], sizeof(char), blocksize, istream);
  }
  
  return (total_samples);
}

float max_amp_t(data, n)
float data[];
long n;
{
	int i;
	float absval, maxamp = 0.0;

	for (i=0; i<n; i++) {
		absval = (data[i] > 0) ? (data[i]) : (-data[i]);
		maxamp = (absval > maxamp) ? (absval) : (maxamp);
	}
	return (maxamp);
}
