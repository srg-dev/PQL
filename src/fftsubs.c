#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/*
	two sets of C routines are used for real data;
		cfour(data,n,isign)
	and
		cfftr(data,n)

	input to cfour is an array of complex values either
	defined as a complex structure or a real array with real
	and imaginary parts alternating.  cfour makes no assumptions
	about the data and isign is either + or - 1 depending on
	whether you are taking a transform or inverse transform.
	cfftr assumes the input data is pure real. 
	To get the correct amplitudes back from cfour after
	inverse transforming divide all values by the number of data
	points n.  To get the correct amplitudes back from cfftri
	divide all values by the number of data points divided by 2.

*/

void cfour( data, n, isign )
int n, isign;
float data[];
{
	int ip0, ip1, ip2, ip3, i3rev;
	int i1, i2a, i2b, i3;
	float sinth, wstpr, wstpi, wr, wi, tempr, tempi, theta;
	double sin();
	ip0=2;
	ip3=ip0*n;
	i3rev=1;
	for( i3=1; i3<=ip3; i3+=ip0 ) {
		if( i3 < i3rev ) {
			tempr = data[i3-1];
			tempi = data[i3];
			data[i3-1] = data[i3rev-1];
			data[i3] = data[i3rev];
			data[i3rev-1] = tempr;
			data[i3rev] = tempi;
			}
		ip1 = ip3 / 2;
		do {
			if( i3rev <= ip1 )
				break;
			i3rev -= ip1;
			ip1 /= 2;
			}
			while ( ip1 >= ip0 );
		i3rev += ip1;
	}

	ip1 = ip0;
	while ( ip1 < ip3 ) {
			ip2 = ip1 * 2;
			theta = 6.283185/( (float) (isign*ip2/ip0) );
			sinth = (float) sin( (double) (theta/2.) );
			wstpr = -2.*sinth*sinth;
			wstpi = (float) sin( (double) theta );
			wr = 1.;
			wi = 0.;
			for ( i1=1; i1<=ip1; i1+=ip0 ) {
			for ( i3=i1; i3<ip3; i3+=ip2 ) {
				i2a=i3;
				i2b=i2a+ip1;
				tempr = wr*data[i2b-1] - wi*data[i2b];
				tempi = wr*data[i2b] + wi*data[i2b-1];
				data[i2b-1] = data[i2a-1] - tempr;
				data[i2b] = data[i2a] - tempi;
				data[i2a-1] += tempr;
				data[i2a] += tempi;
				}
				tempr = wr;
				wr = wr*wstpr - wi*wstpi + wr;
				wi = wi*wstpr + tempr*wstpi + wi;
				}
			ip1=ip2;
	}
return;
}

void cfftr( float *x, int n )
{
	int nn, is, nm, j, i;
	int k1j, k1i, k2j, k2i;
	float s, fn, ex, wr, wi, wwr, wrr, wwi, a1, a2, b1, b2;
	double sin(), cos();
	nn = n/2;
	is = 1;
	cfour( x, nn, is );
	nm = nn/2;
	s = x[0];
	x[0] += x[1];
	x[n] = s - x[1];
	x[1] = 0.0 ;
	x[n+1] = 0.0;
	x[nn+1] = (-x[nn+1]);
	fn = (float) n;
	ex = 6.2831853 / fn;
	j = nn;
	wr = 1.0;
	wi = 0.0;
	wwr = (float) cos( (double) ex );
	wwi = (float) (-sin( (double) ex ));
	for (i=2; i<=nm; i++) {
		wrr = wr*wwr-wi*wwi;
		wi = wr*wwi+wi*wwr;
		wr = wrr;
		k1j = 2*j-1;
		k1i = 2*i-1;
		k2j = 2*j;
		k2i = 2*i;
		a1 = 0.5*(x[k1i-1]+x[k1j-1]);
		a2 = 0.5*(x[k2i-1]-x[k2j-1]);
		b1 = 0.5*(-x[k1i-1]+x[k1j-1]);
		b2 = 0.5*(-x[k2i-1]-x[k2j-1]);
		s = b1;
		b1 = b1*wr+b2*wi;
		b2 = b2*wr-s*wi;
		x[k1i-1] = a1-b2;
		x[k2i-1] = (-a2-b1);
		x[k1j-1] = a1+b2;
		x[k2j-1] =  a2-b1;
		j -= 1;
	}
	return;
}

void cfftri( x, n )
int n;
float x[];
{
	int nn, is, nm, j, i, k1j, k1i, k2j, k2i;
	float s, fn, ex, wr, wi, wwr, wwi, wrr, a1, a2, b1, b2;
	double sin(), cos();
	nn = n/2;
	s = x[0];
	x[0] = 0.5 * ( x[0] + x[n] );
	x[1] = 0.5 * ( s - x[n] );
	x[nn+1] = (-x[nn+1]);
	is = -1;
	nm = nn/2;
	fn = (float) n ;
	ex = 6.2831853 / fn ;
	j = nn;
	wr = 1.0;
	wi = 0.0;
	wwr = (float) cos ( (double) ex );
	wwi = (float) ( - sin( (double) ex ) );
	for ( i=2; i<=nm; i++ ) 
	{
		wrr = wr*wwr-wi*wwi;
		wi = wr*wwi+wi*wwr;
		wr = wrr;
		k1j = 2*j-1;
		k1i = 2*i-1;
		k2j = 2*j;
		k2i = 2*i;
		a1 = 0.5 * ( x[k1i-1] + x[k1j-1] );
		a2 = 0.5 * ( x[k2i-1] - x[k2j-1] );
		b1 = 0.5 * (-x[k1i-1] + x[k1j-1] );
		b2 = 0.5 * (-x[k2i-1] - x[k2j-1] );
		s = b1;
		b1 = b1*wr+b2*wi;
		b2 = b2*wr-s*wi;
		x[k1i-1] = (a1 - b2);
		x[k2i-1] = (-a2-b1);
		x[k1j-1] = (a1+b2);
		x[k2j-1] = (a2-b1);
		j -= 1;
	}
	cfour(x, nn, is);
	return;
}

#define PI 3.1415926535897932384626433
/*********************************************************************
 * fft:
 * Discrete FFT for perfect powers of two.
 *
 * The sine tables are calcualted when the fuction is first used and
 * extended when needed for subsequent calls where nfft has
 * increased.
 *
 * direction: (+1)=Forward DFT, (-1)=Inverse DFT
 *
 * There is a minor optimization if this routine is called with the
 * same nfft repeatedly, the sine tables will be not recalculated.
 *
 * This routine can be called with the first two arguments set to 0 to
 * request clean-up of allocated sine table buffers.
 *
 * This is a combination of modified versions of routines from
 * Numerical Utilities (NumUtils), www.xgraph.org/numutil.html
 *
 * Returns 0 on success and -1 on error.
 *********************************************************************/
int
fft ( double real[], double imag[], int nfft, int direction)
{
  static double *sintab1 = 0;
  static double *sintab2 = 0;
  static int sinallocated = 0;
  
  int i, step, j, m, mmax;
  int n2, k;
  double sinphi, phi, coef_i, coef_r, sci, scr;
  double tmpr, tmpi, pi2;
  double treal, timag;
  
  /* Perform clean-up if requested */
  if ( ! real && ! imag )
    {
      if ( sintab1 )
	free (sintab1);
      if ( sintab2 )
	free (sintab2);
      sintab1 = 0;
      sintab2 = 0;
      sinallocated = 0;
    }
  
  if ( direction != 1 && direction != -1 )
    {
      fprintf (stderr, "fft(): direction argument must be 1 (forward) or -1 (inverse)\n");
      return -1;
    }
  
  /* Sanity check fft length */
  m = 2;
  while ( m < nfft )
    {
      m = m << 1;
    }
  if ( m != nfft )
    {
      fprintf (stderr, "fft(): nfft (%d) is not a perfect power of two\n", nfft);
      return -1;
    }
  
  /* Allocate sine tables if needed */
  if ( sinallocated != nfft )
    {
      if ( (sintab1 = (double *) realloc (sintab1, nfft * sizeof(double))) == NULL )
	{
	  fprintf (stderr, "Error allocating memory\n");
	  return -1;
	}
      if ( (sintab2 = (double *) realloc (sintab2, nfft * sizeof(double))) == NULL )
	{
	  fprintf (stderr, "Error allocating memory\n");
	  return -1;
	}
      
      pi2 = 0.5 * PI;
      mmax = 1;
      k = 0;
      
      while ( mmax < nfft )
	{
	  step = mmax << 1;
	  phi = pi2 / (double) mmax;
	  sintab1[k] = sin (phi);
	  sintab2[k] = sin (phi * 2.0);
	  k++;
	  m = 0;
	  while ( m < mmax )
	    {
	      i = m;
	      while ( i < nfft )
		{
		  j = i + mmax;
		  i += step;
		}
	      m++;
	    }
	  
	  mmax = step;
	}
      
      sinallocated = nfft;
    }
  
  n2 = nfft >> 1;
  j = 0;
  i = 0;
  
  /* Perform decimation in time by bit reversing up front */
  while ( i < nfft )
    {
      if ( i < j )
	{
	  treal = real[j]; timag = imag[j];
	  real[j] = real[i]; imag[j] = imag[i];
	  real[i] = treal; imag[i] = timag;
	}
      m = n2;
      while ( (j > m-1) && (m >= 2) )
	{ j -= m;  m = m >> 1; }
      j += m;
      i++;
    }
  
  mmax = 1;
  k = 0;
  while ( mmax < nfft )
    {
      step = mmax << 1;
      sinphi = -1 * (double)direction * sintab1[k];
      sci = -1 * (double)direction * sintab2[k];
      scr = -2.0 * sinphi * sinphi;
      k++;
      m = 0;
      coef_r = 1.0;
      coef_i = 0.0;
      
      while ( m < mmax )
	{
	  i = m;
	  while ( i < nfft )
	    {
	      j = i + mmax;
	      tmpr = (coef_r * real[j]) - (coef_i * imag[j]);
	      tmpi = (coef_r * imag[j]) + (coef_i * real[j]);
	      real[j] = real[i] - tmpr;
	      imag[j] = imag[i] - tmpi;
	      real[i] += tmpr;
	      imag[i] += tmpi;
	      i += step;
	    }
	  
	  tmpr = coef_r;
	  m++;
	  coef_r = coef_r + (coef_r * scr) - (coef_i * sci);
	  coef_i = coef_i + (coef_i * scr) + (tmpr * sci);
	}
      
      mmax = step;
    }
  
  /* A forward transform will scale the values by nfft, compensate. */
  /* This particular code base does not require this compenstation
     if ( direction > 0 )
     {
      double scale = 1.0 / (double) nfft;
      for ( i=0; i < nfft; i++ )
	{
	  real[i] *= scale;
	  imag[i] *= scale;
	}
    }
  */
  
  return 0;
}  /* End of fft() */
