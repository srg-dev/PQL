/*                                                                     */
/*   Program to test filters for PASSCAL Instrument                    */
/*	These filters are Butterworth highpass and lowpass filters     */
/*	both filters may be implemented or only one filter             */
/*	This program limits the number of poles to be either 0, 2 or 4 */
/*	The program to calculate the pole position or to filter does   */
/*	not have any limits                                            */
/*                                                                     */
/*   Input parameters                                                  */
/*     NH = order of the high pass filter can be 0, or an even number
		up to 12                                               */
/*     FH = high pass cutoff frequency                                 */
/*     NL = order of the low pass filter can be 0, or an even number 
		up to 12                                               */
/*     FL = low pass cutoff frequency                                  */
/*     dt = sample rate                                                */
/*                                                                     */
/*  The program calculates the filter poles then generates a linear    */
/*	sweep with beginning frequency = 0 to stop frequency = nyqiist */
/*	This sweep is then filtered by the requested filters.          */
/*	The output is then written to disk in ascii format             */
/*	The file names for the output are sweep.asc and filter.asc     */
/*                                                                     */
/*       written by jcf   feb 1988
	modified  july 1993.                                                             */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "pql_defines.h"
#include "pql_externs.h"

gboolean beepMe(gpointer nil);

typedef struct
    {
	double real ;
	double imag ;
    } iircomplex ;

void highpass(), lowpass(), filt();

int _iir(DATAINFO *fromData, DATAINFO *toData, int active_filter)
{ 
  gint32		*ptr;
  short         *iptr;
  float			*fptr;
  double		*dptr;
  int			NH, NL, i;
  iircomplex	add_c(), sub_c(), cmul_c(), div_c(), polar_c() ;
  iircomplex	conj_c(), mul_c(), cart_c(), sqrt_c() ;
  iircomplex	pl[12], ph[12] ;
  double		FH, FL, dt, b0l, b0h ;
//  double		f1, f0 ;
  double		a1, a2, b1, b2 ;
  double       *af, trcMean, secsPsamp;
  char			*to_data_pp, *from_data_p=fromData->data;
  int			data_form;
  int			numSamples;

  trcMean = fromData->amps.mean;
  data_form = fromData->head.data_form;
  numSamples = fromData->head.numSamples;
  secsPsamp = fromData->head.secsPsamp;
  to_data_pp = toData->data;
  
  //    First get high pass filter parameters
  NH = -1 ;
  while((NH != 0) && ((NH%2) != 0) && (NH < 13))
    {
      NH = settings.general.filter.filts[active_filter]->poles[HIGHF];
    }
  /*       get high pass cut-off frequency     */
  FH = 0 ;
  if ( NH != 0)
    {
      while(FH == 0)
        {
	  
	  FH = (settings.general.filter.filts[active_filter]->cutoff[HIGHF]);
	  
	  if(FH == 0) {
		  g_idle_add(beepMe, NULL);
		  states.msg = g_strdup("High Frequency Cutoff = 0, Not Allowed.  "
		  	  					"Please Adjust Value via Controls Panel or Spectra Screen.");
		return FAILURE;
	  }
	}
    }      
  /*             Get low pass filter parameters    */
  NL = -1 ;
  while((NL != 0) && ((NL%2) != 0) && (NL < 13))
    {
      NL = settings.general.filter.filts[active_filter]->poles[LOWF];
    }

  /*       get low pass cut-off frequency     */
  FL = 0 ;
  if ( NL != 0)
    {

	  FL = (settings.general.filter.filts[active_filter]->cutoff[LOWF]);
	    
      if(FL == 0) {
		  g_idle_add(beepMe, NULL);
		  states.msg = g_strdup("High Frequency Cutoff = 0, Not Allowed.  Please Adjust Value.");
		  return FAILURE;
      }
      else {

	FL = 0 ;

	if ( FH != 0)
	  {
	    
	    /*      while(FH >= FL) */
	    while(FH > FL)
	      {
		
		FL = (settings.general.filter.filts[active_filter]->cutoff[LOWF]);
		if (FL > ((1/secsPsamp)/2)) {
		  NL = 0;
		  g_idle_add(beepMe, NULL);
		  states.msg = g_strdup_printf("Low Frequency Cutoff Must Be <= %f.  Please Adjust Value.", 
		  	  							(float) 1./secsPsamp/2.);
		  return FAILURE;
		}
		
		if(FL == 0) {
		  g_idle_add(beepMe, NULL);
		  states.msg = g_strdup("Low Frequency Cutoff = 0, Not Allowed.  Please Adjust Value.");
		  return FAILURE;
		}
		
		
		if(FL < FH) {
		  g_idle_add(beepMe, NULL);
		  states.msg = g_strdup("Low Frequency Cutoff Must Be > High Cutoff Frequency.  Please Adjust Values.");
		  return FAILURE;
		}
	      } /* end while */
	  } /* end if */
	else /* FH == 0 */
	  {
		FL = (settings.general.filter.filts[active_filter]->cutoff[LOWF]);
		
		if (FL > ((1/secsPsamp)/2)) {
		  NL = 0;
		  g_idle_add(beepMe, NULL);
		  states.msg = g_strdup_printf("Low Frequency Cutoff Must Be <= %f.  Please Adjust Value.", 
		  	  							(float) 1./secsPsamp/2.);
		  return FAILURE;
		}
		
		if(FL == 0) {
		  g_idle_add(beepMe, NULL);
		  states.msg = g_strdup("Low Cutoff Frequency = 0, Not Allowed.  Please Adjust Value.");
		  return FAILURE;
		}
	    
	  }
      } /* end else */
    }
  
  /*      Get sample rate   */
  
  dt = 0 ;
  while (dt <= 0)
    {
      dt = secsPsamp;
    }
  
  //         Get highpass filter poles if necessary
  if(NH != 0)
    {
      highpass(FH,dt,NH,ph,&b0h) ;
    }
  
  //      Get low pass filter poles if necessary
  if(NL != 0)
    {
      lowpass(FL,dt,NL,pl,&b0l) ;
    }
  
//  f0 = 0. ;     /*  start frequency    */
//  f1 = 1./(dt*2.) ;	/*  stop frequency = nyquist  */
  
  if ((af = (double *)calloc(numSamples,sizeof(double))) == NULL) {
    fprintf(stderr,"Out of memory.\n");
    exit(-1);
  }  
  
  switch (data_form) 
  {	// demean the data
  case BIT32:
    ptr = (gint32 *)from_data_p;
    for (i=0; i<numSamples; i++ ) 
	{
      af[i] = (float) (ptr[i] -  trcMean);
    }
    break;
    
  case BIT16:
    iptr = (short *)from_data_p;
    for (i=0; i<numSamples; i++ ) 
	{
      af[i] = (float) (iptr[i] -  trcMean);
    }
    break;
    
  case MY_FLOAT:
    fptr = (float *)from_data_p;
    for (i=0; i<numSamples; i++ ) 
	{
      af[i] = (float) (fptr[i] -  trcMean);
    }
    break;
    
  case MY_DOUBLE:
    dptr = (double *)from_data_p;
    for (i=0; i<numSamples; i++ ) 
	{
      af[i] = (float) (dptr[i] -  trcMean);
    }
    break;
  }
  
  //	now start filtering the data
  //    Filter is implemented as a cascade of second order filters
  //	high pass filter first use poles ph
  //    Numerator polynomial is z**2 - 2*z + 1

  
  if(NH != 0)
    {
      for ( i=0 ; i<NH ; i +=2)
	{
	  
	  /*	get first set of second order filter coeficients  */
	  /*      from each pair of poles                           */
	  
	  a1 = -2*ph[i].real ;
	  a2 = ph[i].real*ph[i].real + ph[i].imag*ph[i].imag ;
	  b1 = -2 ;
	  b2 = 1 ;
	  
	  filt (a1, a2, b1, b2, numSamples, af, af) ;
	  
	}
      /*        apply gain section          */
      for ( i=0 ; i<numSamples ; i++)
	{
	  af[i] = b0h*af[i] ;
	}
    }
  
  //      apply low pass filter using poles pl
  //	Numerator polynomial is z**2 + 2*z + 1

  if ( NL != 0)
    {
    for ( i=0 ; i<NL ; i +=2)
	{
	  
	  a1 = -2*pl[i].real ;
	  a2 = pl[i].real*pl[i].real + pl[i].imag*pl[i].imag ;
	  b1 = 2 ;
	  b2 = 1 ;
	  
	  filt (a1, a2, b1, b2, numSamples, af, af) ;

	}
      
    // apply gain section
    for ( i=0 ; i<numSamples ; i++)
	{
	  af[i] = b0l*af[i] ;
	}
    }
  
  {
	double *minPtr = &toData->amps.min;
	double *maxPtr = &toData->amps.max;
	double *meanPtr = &toData->amps.mean, mean=0;
	double *diffPtr = &toData->amps.diff;

	fptr = (float*)(to_data_pp);
	*minPtr = *maxPtr = af[0];
    for (i=0; i< numSamples; i++ ) 
	{
		fptr[i] = (float)af[i];
		mean += fptr[i];
		if (fptr[i] > *maxPtr)
		{
			*maxPtr = fptr[i];
			continue;
		}
		if (fptr[i] < *minPtr)
			*minPtr = fptr[i];
    }
	*meanPtr = mean/numSamples;
	*diffPtr = *maxPtr - *minPtr;
  }
      
  free(af);

  return PQLXSUCCESS;
}

/**********************************************************************/
/*          add_c                                                     */
/**********************************************************************/
/*
    Routine to add two iircomplex numbers

	w = add_c(u,v)
        w = u + v
*/

iircomplex	    add_c (u,v)
					iircomplex	    u ;
					iircomplex	    v;
{
    iircomplex	w ;

    w.real = u.real + v.real ;
    w.imag = u.imag + v.imag ;

    return (w) ;
}

/***********************************************************************/
/*             mul_c                                                   */
/***********************************************************************/
/*
	Routine to multiply two iircomplex numbers

	        w = mul_c(u,v)
		w = u * v

*/
iircomplex	mul_c (u,v)
iircomplex		u ;
iircomplex		v ;
{
	iircomplex		w ;

	w.real = u.real*v.real - u.imag*v.imag ;
	w.imag = u.real*v.imag + u.imag*v.real ;

	return (w) ;
}

/***********************************************************************/
/*            cmul_c (a,u)                                             */
/***********************************************************************/
/*
    Routine to multiply a real number times a iircomplex number

	w = cmul_c (a,u)

	a - real number
	u - iircomplex number
*/
iircomplex	    cmul_c (a,u)
double	    a ;
iircomplex	    u ;
{
    iircomplex	w ;

    w.real = a * u.real ;
    w.imag = a * u.imag ;

    return (w) ;
}

/*************************************************************************/
/*               sub_c                                                   */
/*************************************************************************/
/*
    Routine to subtract two iircomplex numbers

	w = sub_c(u,v)
	w = u - v
*/
iircomplex	sub_c(u,v)
iircomplex	    u ;
iircomplex	    v ;
{
    iircomplex	w ;
    w.real = u.real - v.real ;
    w.imag = u.imag - v.imag ;

    return (w) ;
}

/******************************************************************/
/*                    div_c                                       */
/******************************************************************/
/*
    Routine to divide two iircomplex numbers

	w = div_c(u,v)
	w = u/v

*/
iircomplex	div_c(u,v)
iircomplex	    u ;
iircomplex	    v ;
{
    iircomplex	w ;

    /*   check for divide by 0    */
    if (!(v.real == 0 && v.imag == 0))						// bug #256
    {
		w.real = ((u.real * v.real) + (u.imag * v.imag)) /
					((v.real * v.real) + (v.imag * v.imag)) ;
		w.imag = ((u.imag * v.real) - (u.real * v.imag)) /
					((v.real * v.real) + (v.imag * v.imag)) ;

		return (w) ;
    }
    else
    {
		fprintf (stderr, "ERROR: iircomplex division by 0 in div_c\n") ;
		exit (1) ;
    }
}

/***************************************************************/
/*                 conj_c                                      */
/***************************************************************/
/*
	Routine to calculate the iircomplex conjugate

		w = conjugate(u)

*/
iircomplex	conj_c(u)
iircomplex		u ;
{
	iircomplex		w ;

	w.real = u.real ;
	w.imag = -u.imag ;

	return (w) ;
}

#if 0
/*******************************************************************/
/*             sweep (f1, f0, npts, dt, as)                        */
/*******************************************************************/
/*	Routine to calculate linear frequency sweep
	    f1 = stop frequency
	    f0 = start frequency
	    npts = number of points in sweep
	    dt = time between samples
	    as = array for storage of data

*/
void sweep(f1, f0, npts, dt, as)
double	f1, f0, dt, as[] ;
int	npts ;
{
    double  tf, df, tt ;
    int i ;

    /*    PI = 3.1415927 ; */
    tf = npts*dt ;  /* get final time */
    df = 2.*G_PI*(f1-f0)/(tf*2) ;	/*  frequency increment  */
    for( i=0 ; i<npts ; i++)
    {
	tt = i*dt ;
	as[i] = sin(f0*tt + df*tt*tt) ;
    }

}
#endif

/************************************************************************/
/*        filt (a1, a2, b1, b2, npts, fi, fo)                           */
/************************************************************************/
/*	Routine to apply a second order recursive filter to the data
	denomonator polynomial is z**2 + a1*z + a2
	numerator polynomial is z**2 + b1*z + b2
	    fi = input array
	    fo = output array
	    npts = number of points

*/
void filt (a1, a2, b1, b2, npts, fi, fo)
double	a1, a2, b1, b2 ;
double	fi[], fo[] ;
int	npts ;
{
    double  d1, d2, out ;
    int i ;

    d1 = 0 ;
    d2 = 0 ;
    for ( i=0 ; i<npts ; i++)
    {
	out = fi[i] + d1 ;
	d1 = b1*fi[i]  - a1*out + d2 ;
	d2 = b2*fi[i] - a2*out ;
	fo[i] = out ;
    }

}

/***************************************************************/
/*              wdat(FF, npts, dt, ar)                         */
/***************************************************************/
/*
    Routine to write data in format for asci2sac to read
	FF = output file name
	npts = number of points to output
	dt = sample rate in seconds
	ar = array of values to output

*/
void wdat (FF, npts, dt, ar)
char	   FF[] ;
int	    npts ;
double	    dt ;
double	    ar[] ;
{
    int	    i ;
    FILE    *fn ;

    if((fn = fopen(FF,"w+")) == NULL)
    {
	printf("cannot create output file ") ;
	exit (1) ;
    }
    
    for ( i=0 ; i<npts ; i++)
    {
	fprintf(fn, "%16.8le\n", ar[i]) ;
    }
    fclose(fn) ;
}

/**************************************************************************/
/*                   lowpass (fc,dt,n,p,b)                               */
/**************************************************************************/
/*
    Routine to compute lowpass filter poles for Butterworth filter 
	fc = desired cutoff frequency
	dt = sample rate in seconds
	n = number of poles (MUST BE EVEN)
	p = pole locations (RETURNED)
	b = gain factor for filter (RETURNED)

*/
/*   Program calculates a continuous Butterworth low pass IIRs with required */
/*    cut off frequency.                                                     */
/*   This program is limited to using an even number of poles                */
/*   Then a discrete filter is calculated utilizing the bilinear transform   */
/*   Methods used here follow those in Digital Filters and Signal Processing */
/*   by Leland B. Jackson  */

void lowpass(fc,dt,n,p,b)
double	    fc, dt, *b ;
iircomplex	    p[] ;
int	    n ;
{  
    double	wcp, wc, b0 ;
    int		i, i1 ;
    iircomplex	add_c(), mul_c(), div_c(), cmul_c(), sub_c() ;
    iircomplex	conj_c() ;
    iircomplex	one, x, y ;

/*			    Initialize variables       */
/*    PI = 3.1415927 ; */
    wcp = 2 * fc * G_PI ;
    wc = (2./dt)*tan(wcp*dt/2.) ;
    one.real = 1. ;
    one.imag = 0. ;
    for (i=0 ; i<n ; i += 2)
    {
/*               Calculate position of poles for continuous filter    */

	i1 = i + 1 ;
        p[i].real = -wc*cos(i1*G_PI/(2*n)) ;
	p[i].imag = wc*sin(i1*G_PI/(2*n)) ;
	p[i+1] = conj_c(p[i]) ;
    }
    for ( i=0 ; i<n ; i += 2)
    {
/*             Calculate position of poles for discrete filter using    */
/*              the bilinear transformation                             */

	p[i] = cmul_c(dt/2,p[i]) ;
	x = add_c(one,p[i]) ;
	y = sub_c(one,p[i]) ;
	p[i] = div_c(x,y) ;
	p[i+1] = conj_c(p[i]) ;
    }

/*	calculate filter gain   */

    b0 = 1. ;
    for (i=0 ; i<n ; i +=2)
    {
	x = sub_c(one,p[i]) ;
	y = sub_c(one,p[i+1]) ;
	x = mul_c(x,y) ;
	b0 = b0*4./x.real ;
    }
    b0 = 1./b0 ;
    *b = b0 ;
}

/**************************************************************************/
/*                   highpass (fc,dt,n,p,b)                               */
/**************************************************************************/
/*
    Routine to compute lowpass filter poles for Butterworth filter 
	fc = desired cutoff frequency
	dt = sample rate in seconds
	n = number of poles (MUST BE EVEN)
	p = pole locations (RETURNED)
	b = gain factor for filter (RETURNED)

*/
/*   Program calculates a continuous Butterworth highpass IIRs               */
/*   First a low pass filter is calculated with required cut off frequency.  */
/*   Then this filter is converted to a high pass filter                     */
/*   This program is limited to using an even number of poles                */
/*   Then a discrete filter is calculated utilizing the bilinear transform   */
/*   Methods used here follow those in Digital Filters and Signal Processing */
/*   by Leland B. Jackson  */

void highpass(fc,dt,n,p,b)
double	    fc, dt, *b ;
iircomplex	    p[] ;
int	    n ;
{  
    double	wcp, wc,  alpha, b0 ;
    int		i ;
    iircomplex	add_c(), mul_c(), div_c(), cmul_c(), sub_c() ;
    iircomplex	conj_c() ;
    iircomplex	one, x, y ;

/*         Initialize variables          */
/*     PI = 3.1415927 ; */
    wcp = 2 * fc * G_PI ;
    wc = (2./dt)*tan(wcp*dt/2.) ;
    alpha = cos(wc*dt) ;
    one.real = 1. ;
    one.imag = 0. ;

/*            get poles for low pass filter     */

    lowpass(fc,dt,n,p,&b0) ;

/*       now find poles for highpass filter      */

    for (i=0 ; i<n ; i+=2)
    {
	x = cmul_c (alpha,one) ;
	x = sub_c (x,p[i]) ;
	y = cmul_c (alpha,p[i]) ;
	y = sub_c(one,y) ;
	p[i] = div_c(x,y) ;
	p[i+1] = conj_c(p[i]) ;
    }

/*      Calculate gain for high pass filter    */

    b0 = 1. ;
    for (i=0 ; i<n ; i += 2)
    {
	x = add_c(one,p[i]) ;
	y = add_c(one,p[i+1]) ;
	x = mul_c(x,y) ;
	b0 = b0*4./x.real ;
    }					    
    b0 = 1./b0 ;
    *b = b0 ;
}
