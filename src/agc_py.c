/*   agc_py.c   */

/*
 *      Major portions stolen from John N. Louie, UNR
 *
 *	automatic gain control with optional event detection
 *	input data is in rows of t smoothed over window points
 *	the first arrival is detected when a moving dwind exceeds an initial
 *	dwind by a threshhold factor.
 *
 *      Steve Azevedo, October 2001
 */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <glib.h>
#include "agc_py.h"
#include "pql_defines.h"
#include "pql_externs.h"

AGCINFO agcinfo;
FILTEREDMEM filteredmem;

void
initAgcInfo (void)
{
  agcinfo.window = settings.general.filter.AGC.winLen;
  agcinfo.dwind = settings.general.filter.AGC.dwin;
  agcinfo.detect = settings.general.filter.AGC.detect;
  agcinfo.threshold = settings.general.filter.AGC.threshold;
}

#if 0
void initFilteredMem (void)
{
  filteredmem.filteredPointer = NULL;
  filteredmem.n = 0;
}

void setAgcInfo (int window, int dwind, int detect, float threshold)
{
  agcinfo.window = window;
  agcinfo.dwind = dwind;
  agcinfo.detect = detect;
  agcinfo.threshold = threshold;
}
#endif

/*
 *   trace = the seismic trace
 *   n = number of samples in trace
 */
//float * agc (float *trace, int n)
int agc (/*DATAINFO *fromData, */DATAINFO *toData)
{
  int nt, i;
  float initaver, probe, l1norm, oldnorm;
  float *x, *y, *xp, *yp, *tail, *head, *start, *end;
#if 0
  gint32 *ptr;
  short *iptr;
  float *fptr;
  double *dptr;

//  nt = n;
//  x = trace;
//  y = (float *) alloc (nt, sizeof (float));
#endif  
  initAgcInfo();
  
//  data_form = toData->head.data_form;
  nt = toData->head.numSamples;
  x = (float *) calloc (nt, sizeof (float));
  y = (float *) toData->data;
  
  memcpy(x, y, sizeof(float) * nt);
  y = y + (nt);
  x = x + (nt);

#if 0
  switch (data_form) 
  {	// convert the data to float
  case BIT32:
    ptr = (gint32 *)fromData->data;
    for (i=0; i<nt; i++ ) 
	{
      y[i] = x[i] = (float) (ptr[i]);
    }
  break;
    
  case BIT16:
    iptr = (short *)fromData->data;
    for (i=0; i<nt; i++ ) 
	{
      y[i] = x[i] = (float) (iptr[i]);
    }
  break;
    
  case MY_FLOAT:
    fptr = (float *)fromData->data;
    for (i=0; i<nt; i++ ) 
	{
      y[i] = x[i] = (float) (fptr[i]);
    }
  break;
    
  case MY_DOUBLE:
    dptr = (double *)fromData->data;
    for (i=0; i<nt; i++ ) 
	{
      y[i] = x[i] = (float) (dptr[i]);
    }
  break;
  }
#endif
  
  filteredmem.filteredPointer = (void *) y;
  filteredmem.n++;

  /* gain each trace by l1 norm of moving window */
  xp = x; yp = y;
  /* 
   * detect first significant energy by comparing 
   * short moving window with an initial average 
   */
  if (agcinfo.detect) {

  DETECTS: 
    initaver = 0.0;
    for (end = x + agcinfo.dwind; xp < end; xp++) {
      if (*xp > 0.) 
	initaver += *xp;
      else 
	initaver -= *xp;
    }

    //yp = y + agcinfo.dwind / 2;
    tail = x;
    head = x + agcinfo.dwind;
    end = x + nt - agcinfo.dwind / 2;
    probe = initaver;

    for (xp = x + agcinfo.dwind / 2; xp < end; xp++) {
      yp++;

      if (probe > (initaver * agcinfo.threshold))
	break;

      if (*head > 0.) 
	probe += *head++;
      else 
	probe -= *head++;
      
      if (*tail > 0.) 
	probe -= *tail++;
      else 
	probe += *tail++;
      
      if ((xp == end) || (probe == 0.)) {
//	fprintf (stderr, "No energy detected in trace (detect)\n");
	goto THEEND;
      }
    }
  }
  
  /* untaper beginning of trace */
  start = xp;
  l1norm = 0.0;
  for (end = start + agcinfo.window; xp < end; xp++) {
    if (*xp > 0.) 
      l1norm += *xp;
    else 
      l1norm -= *xp;
  }
  //fprintf (stderr, "Taper: %f Window: %d", l1norm, agcinfo.window);
  if (l1norm == 0) {
    xp = x;
    goto DETECTS;
  }

  end = start + agcinfo.window / 2;
  for (xp = start; xp < end; xp++) 
    *yp++ = *xp / l1norm;

  /* core routine */
  tail = start;
  head = start + agcinfo.window;
  oldnorm = l1norm;
  for (end = x + nt - agcinfo.window / 2; xp < end; xp++) {

    if (*head > 0.0) 
      l1norm += *head++;
    else 
      l1norm -= *head++;
    
    if (*tail > 0.0) 
      l1norm -= *tail++;
    else 
      l1norm += *tail++;

    if (oldnorm == 0.0) 
      l1norm = oldnorm;
    
    oldnorm = l1norm;
    *yp++ = *xp / l1norm;
    
  }
    
  /* untaper end of trace */
  for (end = x + nt; xp < end;) 
    *yp++ = *xp++ / l1norm;
  
  {	// set the max/min/mean/diff values
	double *minPtr = &toData->amps.min;
	double *maxPtr = &toData->amps.max;
	double *meanPtr = &toData->amps.mean, mean=0;
	double *diffPtr = &toData->amps.diff;

	*minPtr = *maxPtr = y[0];
    for (i=0; i< nt; i++ ) 
	{
		mean += y[i];
		if (y[i] > *maxPtr)
		{
			*maxPtr = y[i];
			continue;
		}
		if (y[i] < *minPtr)
			*minPtr = y[i];
    }
	*meanPtr = mean/nt;
	*diffPtr = *maxPtr - *minPtr;
  }

 THEEND:

  return PQLXSUCCESS;
  
}

#if 0
/*
 *	allocation with error detection
 */
void *
alloc (int n, int s)
{
  void *ptr;

  if ((ptr = (void *) calloc ((size_t) n, (size_t) s)) <= 0) {
    fprintf (stderr, "Can't allocate %d bytes\n", s);
    ptr = NULL;
  }

  return (ptr);

}

/*
 *   Free memory
 */
void
freeFiltered (void)
{
  if (filteredmem.filteredPointer != NULL) {
    free (filteredmem.filteredPointer);
    if (filteredmem.n > 1)
      fprintf (stderr, "Warning: Unfreed memory in agc!\n");
    filteredmem.n--;
  }
}
#endif
