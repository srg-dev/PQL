/******************************************************************************
 *                                                                            *
 * convert_vax_data.c - Convert VAX-format data to/from Unix (IEEE) format    *
 *                                                                            *
 *    from_vax_i2()  - Byte swap Integer*2                                    *
 *    from_vax_i4()  - Byte reverse Integer*4                                 *
 *    from_vax_r4()  - 32-bit VAX F_floating to IEEE S_floating               *
 *    from_vax_d8()  - 64-bit VAX D_floating to IEEE T_floating               *
 *    from_vax_g8()  - 64-bit VAX G_floating to IEEE T_floating               *
 *    from_vax_h16() - 128-bit VAX H_floating to IEEE X_floating              *
 *                                                                            *
 *    to_vax_i2()    - Byte swap Integer*2                                    *
 *    to_vax_i4()    - Byte reverse Integer*4                                 *
 *    to_vax_r4()    - 32-bit IEEE S_floating to VAX F_floating               *
 *    to_vax_d8()    - 64-bit IEEE T_floating to VAX D_floating               *
 *    to_vax_g8()    - 64-bit IEEE T_floating to VAX G_floating               *
 *  [ to_vax_h16()   - 128-bit IEEE X_floating to VAX H_floating ]            *
 *                                                                            *
 * (Routines in square brackets are not yet implemented.)                     *
 *                                                                            *
 * All calls take 3 arguments:                                                *
 *                                                                            *
 *    C       declaration  void name( const void *, void *, const int * );    *
 *                                                                            *
 *            usage        name( in_array, out_array, &count );               *
 *                                                                            *
 *    Fortran declaration  Subroutine NAME( in_array, out_array, count )      *
 *                                                                            *
 *            usage        Call NAME( in_array, out_array, count )            *
 *                                                                            *
 * The in_array and out_array parameters may refer to the same object.        *
 *                                                                            *
 *                                                                            *
 * VAXes (as well as the Intel 80x86 family) store integers in 2's complement *
 * format, ordering the bytes in memory from low-order to high-order  (collo- *
 * quially  called  little-endian format).  Most Unix machines (Sun, IBM, HP) *
 * also store integers in 2's complement format, but  use  the  opposite  (so *
 * called big-endian) byte ordering.                                          *
 *                                                                            *
 * A  VAX  integer  is converted to (big-endian) Unix format by reversing the *
 * byte order.                                                                *
 *                                                                            *
 * Most  Unix machines implement the ANSI/IEEE 754-1985 floating-point arith- *
 * metic standard.  VAX and IEEE formats are similar  (after  byte-swapping). *
 * The  high-order bit is a sign bit (s).  This is followed by a biased expo- *
 * nent (e), and then a (usually) hidden-bit normalized mantissa  (m).   They *
 * differ in the number used to bias the exponent, the location of the impli- *
 * cit binary point for the mantissa, and the representation  of  exceptional *
 * numbers (e.g., +/-infinity).                                               *
 *                                                                            *
 * VAX floating-point formats:  (-1)**s * 2**(e-bias) * 0.1m                  *
 *                                                                            *
 *                  31              15              0                         *
 *                   |               |              |                         *
 * F_floating        mmmmmmmmmmmmmmmmseeeeeeeemmmmmmm  bias = 128             *
 * D_floating        mmmmmmmmmmmmmmmmseeeeeeeemmmmmmm  bias = 128             *
 *                   mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm                         *
 * G_floating        mmmmmmmmmmmmmmmmseeeeeeeeeeemmmm  bias = 1024            *
 *                   mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm                         *
 * H_floating        mmmmmmmmmmmmmmmmseeeeeeeeeeeeeee  bias = 16384           *
 *                   mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm                         *
 *                   mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm                         *
 *                   mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm                         *
 *                                                                            *
 * IEEE floating-point formats:  (-1)**s * 2**(e-bias) * 1.m                  *
 *                                                                            *
 *                  31              15              0                         *
 *                   |               |              |                         *
 * S_floating        seeeeeeeemmmmmmmmmmmmmmmmmmmmmmm  bias = 127             *
 * T_floating        seeeeeeeeeeemmmmmmmmmmmmmmmmmmmm  bias = 1023            *
 *                   mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm                         *
 * X_floating        seeeeeeeeeeemmmmmmmmmmmmmmmmmmmm  bias = 1023            *
 *                   mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm                         *
 *                   mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm                         *
 *                   mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm                         *
 *                                                                            *
 * A  VAX floating-point number is converted to IEEE floating-point format by *
 * subtracting (1+VAX_bias-IEEE_bias) from the exponent field to  (1)  adjust *
 * from  VAX  0.1m hidden-bit normalization to IEEE 1.m hidden-bit normaliza- *
 * tion and (2) adjust the bias from VAX format to IEEE  format.   True  zero *
 * [s=e=m=0]  and  dirty  zero  [s=e=0, m<>0] are special cases which must be *
 * recognized and handled separately.                                         *
 *                                                                            *
 * Numbers  whose  absolute value is too small to represent in the normalized *
 * IEEE format illustrated above are converted to subnormal form (e=0,  m>0): *
 * (-1)**s * 2**(1-bias) * 0.m.  Numbers whose absolute value is too small to *
 * represent in subnormal form are set to 0.0  (silent  underflow).   Numbers *
 * whose  absolute value exceeds the largest representable IEEE-format number *
 * are converted to +/-infinity (silent overflow).                            *
 *                                                                            *
 * Note: If the fractional part of the VAX floating-point number is too large *
 *       for the corresponding IEEE floating-point format,  bits  are  simply *
 *       discarded  from  the  right.  Thus, the remaining fractional part is *
 *       truncated, not rounded to the lowest-order bit.                      *
 *                                                                            *
 * A  VAX floating-point reserved operand (s=1, e=0, m=X) causes a SIGFPE ex- *
 * ception to be raised.                                                      *
 *                                                                            *
 * Conversely,  an  IEEE  floating-point number is converted to VAX floating- *
 * point format by  adding  (1+VAX_bias-IEEE_bias)  to  the  exponent  field. *
 * +zero  [s=e=m=0]  and  -zero  [s=1, e=m=0] are special cases which must be *
 * recognized and handled separately.                                         *
 *                                                                            *
 * Numbers  whose  absolute value is too small to represent in the normalized *
 * VAX format illustrated above are set  to  0.0  (silent  underflow).   (VAX *
 * floating-point  format does not support subnormal numbers.)  Numbers whose *
 * absolute value exceeds the largest representable VAX-format number cause a *
 * SIGFPE exception to be raised (overflow).  (VAX floating-point format does *
 * not have reserved bit patterns for infinities and not-a-numbers [NaNs].)   *
 *                                                                            *
 *                                                                            *
 Two  variants  of  convert_vax_data.c  are  available  using  LITTLE_ENDIAN and
 APPEND_UNDERSCORE.  If LITTLE_ENDIAN is defined as 0, then the conversions  are
 performed  for a big-endian machine, i.e., byte reordering is performed for all
 data types.  If LITTLE_ENDIAN is defined as 1, then no reordering is performed.
 (Integers  are identical to VAX format.)  If LITTLE_ENDIAN is not defined, then
 it is defined as 1 if "vax", "__vax", "vms", or "__vms" is defined  (i.e.,  for
 VAX C, GNU C running on a VAX or an Alpha, or DEC C), or "M_I86", "_M_IX86", or
 "_M_ALPHA" is defined (i.e., for Microsoft C or  Microsoft  Visual  C++  on  an
 Intel  80x86 or an Alpha), or "i386" or "__i386" is defined (i.e., for Sun C on
 an Intel 80x86).
 *                                                                            *
 If  APPEND_UNDERSCORE  is  defined,  the entry point names are compiled with an
 underscore appended.  This is required so that they can be called by Fortran in
 cases  where  the  Fortran  compiler appends an underscore to externally called
 routines (e.g., Sun Fortran).
 *                                                                            *
 Normally,  all routines are compiled into a single object module.  To compile a
 single routine into its own module, define MAKE_routine_name, substituting  the
 upper-case  name of the routine for routine_name.  For example, MAKE_TO_VAX_I2.
 (This is useful, for example, to insert the routines into a library such that a
 linker may extract only the routines actually needed by a particular program.)
 *                                                                            *
 convert_vax_data.c  assumes an ANSI C compiler, 8-bit chars, 16-bit shorts, and
 32-bit ints.
 *                                                                            *
 *                                                                            *
 * Author:  Lawrence M. Baker                                                 *
 *          U.S. Geological Survey                                            *
 *          345 Middlefield Road  MS977                                       *
 *          Menlo Park, CA  94025                                             *
 *          baker@usgs.gov                                                    *
 *                                                                            *
 *                                                                            *
 *                                 Disclaimer                                 *
 *                                                                            *
 * Although  this  program  has  been  used by the U.S. Geological Survey, no *
 * warranty, expressed or implied, is made by the USGS as to the accuracy and *
 * functioning  of  the  program  and related program material, nor shall the *
 * fact of distribution constitute any such warranty, and  no  responsibility *
 * is assumed by the USGS in connection therewith.                            *
 *                                                                            *
 *                                                                            *
 * Modification History:                                                      *
 *                                                                            *
 *  8-Sep-1992  L. M. Baker      Original version.
 * 12-Jan-1993  L. M. Baker      Convert Fortran data conversion routines to C.
 *                               Force underflows to 0 (as the VAX hardware does)
 *                                  to avoid IEEE Not-a-Numbers (NaNs).
 * 14-Jan-1993  L. M. Baker      Define forward and backward conversions for all
 *                                  numeric data types (not all implemented yet).
 * 20-Jan-1993  L. M. Baker      Convert VAX extrema to subnormal/infinities.
 * 22-Jan-1993  L. M. Baker      Allow for little-endian and big-endian machines.
 *                               Define register variables for Microsoft C (2 max).
 * 25-Jan-1993  L. M. Baker      Provide for Fortran naming with underscores.
 *                               Provide for separate compilation for libraries.
 * 27-Jan-1993  L. M. Baker      Swap (16-bit) words in floating-point formats for
 *                                  LITTLE_ENDIAN machines.
 * 16-May-2000  L. M. Baker      Add conditionals for DEC C, Microsoft Visual C++.
 *                               Convert VAX dirty zero (s=e=0, m<>0) to true zero.
 *                               raise( SIGFPE ) for VAX reserved operands.
 *                               Implement to_vax_r4(), to_vax_d8(), to_vax_g8().
 *                               Add const specifier where appropriate.
 *  1-Feb-2001  L. M. Baker      Change long to int (long's are 64 bits on Compaq's
 *                                  Tru64 UNIX)
 *                               Add __alpha to the list of predefined macros that
 *                                  set LITTLE_ENDIAN to 1.
 *  5-Feb-2001  L. M. Baker      Add prototypes for all functions (Matlab's MrC
 *                                  command on Macintosh requires them).
 *  9-Mar-2001  L. M. Baker      #include "convert_vax_data.h".               *
 *                                                                            *
 ******************************************************************************/

#include "convert_vax_data.h"	/* UPCASE, APPEND_UNDERSCORE, FORTRAN_LINKAGE */

#ifndef LITTLE_ENDIAN

/* VAX C, GNU C running on a VAX or an Alpha, or DEC C */

#if defined(vax) || defined(__vax) || defined(vms) || defined(__vms) || \
    defined(__alpha)
#define LITTLE_ENDIAN 1
#endif

/* Microsoft 80x86 C or Microsoft Visual C++ on an 80x86 or an Alpha */

#if defined(M_I86) || defined(_M_IX86) || defined(__M_ALPHA)
#define LITTLE_ENDIAN 1
#endif

/* Sun C on an 80x86 */

#if defined(i386) || defined(__i386)
#define LITTLE_ENDIAN 1
#endif

/* Otherwise, assume big-endian machine */

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 0
#endif

#endif

#if !defined(MAKE_FROM_VAX_I2) && !defined(MAKE_FROM_VAX_I4) && \
    !defined(MAKE_FROM_VAX_R4) && !defined(MAKE_FROM_VAX_D8) && \
    !defined(MAKE_FROM_VAX_G8) && !defined(MAKE_FROM_VAX_H16) && \
    !defined(MAKE_TO_VAX_I2) && !defined(MAKE_TO_VAX_I4) && \
    !defined(MAKE_TO_VAX_R4) && !defined(MAKE_TO_VAX_D8) && \
    !defined(MAKE_TO_VAX_G8) && !defined(MAKE_TO_VAX_H16)

#define MAKE_FROM_VAX_I2
#define MAKE_FROM_VAX_I4
#define MAKE_FROM_VAX_R4
#define MAKE_FROM_VAX_D8
#define MAKE_FROM_VAX_G8
#define MAKE_FROM_VAX_H16

#define MAKE_TO_VAX_I2
#define MAKE_TO_VAX_I4
#define MAKE_TO_VAX_R4
#define MAKE_TO_VAX_D8
#define MAKE_TO_VAX_G8
#define MAKE_TO_VAX_H16

#endif

#include <limits.h>
#if defined(MAKE_FROM_VAX_R4) || defined(MAKE_FROM_VAX_D8) || \
    defined(MAKE_FROM_VAX_G8) || defined(MAKE_FROM_VAX_H16) || \
    defined(MAKE_TO_VAX_R4) || defined(MAKE_TO_VAX_D8) || \
    defined(MAKE_TO_VAX_G8) || defined(MAKE_TO_VAX_H16)
#include <signal.h>
#endif

#if UCHAR_MAX != 255U || USHRT_MAX != 65535U || UINT_MAX != 4294967295U
#error convert_vax_data.c requires 8-bit chars, 16-bit shorts, and 32-bit ints
#endif

#if !defined(__STDC__) && !defined(__cplusplus)  /* const is ANSI C, C++ only */
#define const
#endif

/************************************************************** from_vax_i2() */

#ifdef MAKE_FROM_VAX_I2

void FORTRAN_LINKAGE from_vax_i2( const void *inbuf, void *outbuf,
                                  const int *ntrans ) {

#if LITTLE_ENDIAN

   register const unsigned short *in;   /* Microsoft C: up to 2 register vars */
   register unsigned short *out;        /* Microsoft C: up to 2 register vars */
   int n;


   in  = (const unsigned short *) inbuf;
   out = (unsigned short *) outbuf;

   if ( in != out )
      for ( n = *ntrans; n > 0; n-- )
         *out++ = *in++;

#else

   int n;
   const unsigned char *in;
   unsigned char *out, c1;


   in  = (const unsigned char *) inbuf;
   out = (unsigned char *) outbuf;

   for ( n = *ntrans; n > 0; n-- ) {
      c1     = *in++;
      *out++ = *in++;
      *out++ = c1;
   }

#endif

}

#endif /* #ifdef MAKE_FROM_VAX_I2 */

/************************************************************** from_vax_i4() */

#ifdef MAKE_FROM_VAX_I4

void FORTRAN_LINKAGE from_vax_i4( const void *inbuf, void *outbuf,
                                  const int *ntrans ) {

#if LITTLE_ENDIAN

   register const unsigned int *in;     /* Microsoft C: up to 2 register vars */
   register unsigned int *out;          /* Microsoft C: up to 2 register vars */
   int n;


   in  = (const unsigned int *) inbuf;
   out = (unsigned int *) outbuf;

   if ( in != out )
      for ( n = *ntrans; n > 0; n-- )
         *out++ = *in++;

#else

   int n;
   const unsigned char *in;
   unsigned char *out, c1, c2;


   in  = (unsigned char *) inbuf;
   out = (unsigned char *) outbuf;

   for ( n = *ntrans; n > 0; n-- ) {
      c1     = *in++;
      c2     = *in++;
      *out++ = *++in;
      *out++ = *--in;
      in    += 2;
      *out++ = c2;
      *out++ = c1;
   }

#endif

}

#endif /* #ifdef MAKE_FROM_VAX_I4 */

/************************************************************** from_vax_r4() */

#ifdef MAKE_FROM_VAX_R4

#define SIGN_BIT       0x80000000
#define VAX_EXPONENT   0x7F800000
#define VAX_HIDDEN_BIT 0x00800000
#define VAX_MANTISSA   0x007FFFFF
#define VAX_BIAS       128
#define IEEE_BIAS      127

void FORTRAN_LINKAGE from_vax_r4( const void *inbuf, void *outbuf,
                                  const int *ntrans ) {

#if LITTLE_ENDIAN
   register const unsigned short *in;   /* Microsoft C: up to 2 register vars */
   union { unsigned short i[2]; unsigned int l; } part;
#else
   const unsigned char *in;
   union { unsigned char c[4]; unsigned int l; } part;
#endif
   register unsigned int *out;          /* Microsoft C: up to 2 register vars */
   unsigned int part1;
   int n;
   int e;


#if LITTLE_ENDIAN
   in  = (const unsigned short *) inbuf;
#else
   in  = (const unsigned char *) inbuf;
#endif
   out = (unsigned int *) outbuf;

   for ( n = *ntrans; n > 0; n-- ) {
#if LITTLE_ENDIAN
      part.i[1] = *in++;
      part.i[0] = *in++;
#else
      part.c[1] = *in++;
      part.c[0] = *in++;
      part.c[3] = *in++;
      part.c[2] = *in++;
#endif
      part1 = part.l;
      if ( ( part1 & VAX_EXPONENT ) == 0 ) {
         if ( ( part1 & SIGN_BIT ) == SIGN_BIT )
            raise( SIGFPE );                    /* VAX reserved operand fault */
         *out++ = 0;                                    /* Dirty or true zero */
      } else
         if ( ( e = ( ( part1 & VAX_EXPONENT ) >> 23 ) -
                    ( 1 + VAX_BIAS-IEEE_BIAS ) ) > 0 )          /* Normalized */
            *out++ = part1 - ( ( 1 + VAX_BIAS - IEEE_BIAS ) << 23 );
         else                                                    /* Subnormal */
            *out++ = ( part1 & SIGN_BIT ) | ( ( VAX_HIDDEN_BIT |
                        ( part1 & VAX_MANTISSA ) ) >> ( 1 - e ) );
   }

}

#undef SIGN_BIT
#undef VAX_EXPONENT
#undef VAX_HIDDEN_BIT
#undef VAX_MANTISSA
#undef VAX_BIAS
#undef IEEE_BIAS

#endif /* #ifdef MAKE_FROM_VAX_R4 */

/************************************************************** from_vax_d8() */

#ifdef MAKE_FROM_VAX_D8

#define SIGN_BIT       0x80000000
#define VAX_EXPONENT   0x7F800000
#define VAX_MANTISSA   0x007FFFFF
#define VAX_BIAS       128
#define IEEE_BIAS      1023

void FORTRAN_LINKAGE from_vax_d8( const void *inbuf, void *outbuf,
                                  const int *ntrans ) {

#if LITTLE_ENDIAN
   register const unsigned short *in;   /* Microsoft C: up to 2 register vars */
   union { unsigned short i[2]; unsigned int l; } part;
#else
   const unsigned char *in;
   union { unsigned char c[4]; unsigned int l; } part;
#endif
   register unsigned int *out;          /* Microsoft C: up to 2 register vars */
   unsigned int part1, part2;
   int n;


#if LITTLE_ENDIAN
   in  = (const unsigned short *) inbuf;
#else
   in  = (const unsigned char *) inbuf;
#endif
   out = (unsigned int *) outbuf;

   for ( n = *ntrans; n > 0; n-- ) {
#if LITTLE_ENDIAN
      part.i[1] = *in++;
      part.i[0] = *in++;
      part1     = part.l;
      part.i[1] = *in++;
      part.i[0] = *in++;
      part2     = part.l;
#else
      part.c[1] = *in++;
      part.c[0] = *in++;
      part.c[3] = *in++;
      part.c[2] = *in++;
      part1     = part.l;
      part.c[1] = *in++;
      part.c[0] = *in++;
      part.c[3] = *in++;
      part.c[2] = *in++;
      part2     = part.l;
#endif
      if ( ( part1 & VAX_EXPONENT ) == 0 ) {
         if ( ( part1 & SIGN_BIT ) == SIGN_BIT )
            raise( SIGFPE );                    /* VAX reserved operand fault */
         *out++ = 0;                                    /* Dirty or true zero */
         *out++ = 0;
      } else {
         *out++ = ( part1 & SIGN_BIT ) |                        /* Normalized */
                  ( ( ( part1 & ( VAX_EXPONENT | VAX_MANTISSA ) ) >> 3 ) -
                    ( ( 1 + VAX_BIAS - IEEE_BIAS ) << 20 ) );
         *out++ = ( part1 << 29 ) | ( part2 >> 3 );
      }
   }

}

#undef SIGN_BIT
#undef VAX_EXPONENT
#undef VAX_MANTISSA
#undef VAX_BIAS
#undef IEEE_BIAS

#endif /* #ifdef MAKE_FROM_VAX_D8 */

/************************************************************** from_vax_g8() */

#ifdef MAKE_FROM_VAX_G8

#define SIGN_BIT       0x80000000
#define VAX_EXPONENT   0x7FF00000
#define VAX_HIDDEN_BIT 0x00100000
#define VAX_MANTISSA   0x000FFFFF
#define VAX_BIAS       1024
#define IEEE_BIAS      1023

void FORTRAN_LINKAGE from_vax_g8( const void *inbuf, void *outbuf,
                                  const int *ntrans ) {

#if LITTLE_ENDIAN
   register const unsigned short *in;   /* Microsoft C: up to 2 register vars */
   union { unsigned short i[2]; unsigned int l; } part;
#else
   const unsigned char *in;
   union { unsigned char c[4]; unsigned int l; } part;
#endif
   register unsigned int *out;          /* Microsoft C: up to 2 register vars */
   unsigned int part1, part2;
   int e;
   int n;


#if LITTLE_ENDIAN
   in  = (const unsigned short *) inbuf;
#else
   in  = (const unsigned char *) inbuf;
#endif
   out = (unsigned int *) outbuf;

   for ( n = *ntrans; n > 0; n-- ) {
#if LITTLE_ENDIAN
      part.i[1] = *in++;
      part.i[0] = *in++;
      part1     = part.l;
      part.i[1] = *in++;
      part.i[0] = *in++;
      part2     = part.l;
#else
      part.c[1] = *in++;
      part.c[0] = *in++;
      part.c[3] = *in++;
      part.c[2] = *in++;
      part1     = part.l;
      part.c[1] = *in++;
      part.c[0] = *in++;
      part.c[3] = *in++;
      part.c[2] = *in++;
      part2     = part.l;
#endif
      if ( ( part1 & VAX_EXPONENT ) == 0 ) {
         if ( ( part1 & SIGN_BIT ) == SIGN_BIT )
            raise( SIGFPE );                    /* VAX reserved operand fault */
         *out++ = 0;                                    /* Dirty or true zero */
         *out++ = 0;
      } else
         if ( ( e = ( ( part1 & VAX_EXPONENT ) >> 20 ) -
                    ( 1 + VAX_BIAS - IEEE_BIAS ) ) > 0 ) {      /* Normalized */
            *out++ = part1 - ( ( 1 + VAX_BIAS - IEEE_BIAS ) << 20 );
            *out++ = part2;
         } else {                                                /* Subnormal */
            part1  = VAX_HIDDEN_BIT | ( part1 & ( SIGN_BIT | VAX_MANTISSA ) );
            *out++ = ( part1 & SIGN_BIT ) | ( ( part1 & ( VAX_HIDDEN_BIT |
                        VAX_MANTISSA ) ) >> ( 1 - e ) );
            *out++ = ( part1 << (31+e) ) | ( part2 >> ( 1 - e ) );
         }
   }

}

#undef SIGN_BIT
#undef VAX_EXPONENT
#undef VAX_HIDDEN_BIT
#undef VAX_MANTISSA
#undef VAX_BIAS
#undef IEEE_BIAS

#endif /* #ifdef MAKE_FROM_VAX_G8 */

/************************************************************* from_vax_h16() */

#ifdef MAKE_FROM_VAX_H16

#define SIGN_BIT       0x80000000
#define VAX_EXPONENT   0x7FFF0000
#define VAX_HIDDEN_BIT 0x00010000
#define VAX_MANTISSA   0x0000FFFF
#define VAX_BIAS       16384
#define IEEE_BIAS      1023

void FORTRAN_LINKAGE from_vax_h16( const void *inbuf, void *outbuf,
                                   const int *ntrans ) {

#if LITTLE_ENDIAN
   register const unsigned short *in;   /* Microsoft C: up to 2 register vars */
   union { unsigned short i[2]; unsigned int l; } part;
#else
   const unsigned char *in;
   union { unsigned char c[4]; unsigned int l; } part;
#endif
   register unsigned int *out;          /* Microsoft C: up to 2 register vars */
   unsigned int part1, part2, part3, part4, sign;
   int e;
   int n, shl, shr;


#if LITTLE_ENDIAN
   in  = (const unsigned short *) inbuf;
#else
   in  = (const unsigned char *) inbuf;
#endif
   out = (unsigned int *) outbuf;

   for ( n = *ntrans; n > 0; n-- ) {
#if LITTLE_ENDIAN
      part.i[1] = *in++;
      part.i[0] = *in++;
      part1     = part.l;
      part.i[1] = *in++;
      part.i[0] = *in++;
      part2     = part.l;
      part.i[1] = *in++;
      part.i[0] = *in++;
      part3     = part.l;
      part.i[1] = *in++;
      part.i[0] = *in++;
      part4     = part.l;
#else
      part.c[1] = *in++;
      part.c[0] = *in++;
      part.c[3] = *in++;
      part.c[2] = *in++;
      part1     = part.l;
      part.c[1] = *in++;
      part.c[0] = *in++;
      part.c[3] = *in++;
      part.c[2] = *in++;
      part2     = part.l;
      part.c[1] = *in++;
      part.c[0] = *in++;
      part.c[3] = *in++;
      part.c[2] = *in++;
      part3     = part.l;
      part.c[1] = *in++;
      part.c[0] = *in++;
      part.c[3] = *in++;
      part.c[2] = *in++;
      part4     = part.l;
#endif
      if ( ( part1 & VAX_EXPONENT ) == 0 ) {
         if ( ( part1 & SIGN_BIT ) == SIGN_BIT )
            raise( SIGFPE );                    /* VAX reserved operand fault */
         *out++ = 0;                                    /* Dirty or true zero */
         *out++ = 0;
         *out++ = 0;
         *out++ = 0;
      } else {
         sign  = part1 & SIGN_BIT;
         if ( ( e = ( ( part1 & VAX_EXPONENT ) >> 16 ) -
                    ( 1 + VAX_BIAS - IEEE_BIAS ) ) <= 0 ) {
            part1 = VAX_HIDDEN_BIT | ( part1 & VAX_MANTISSA );   /* Subnormal */

/* The  net right shift of the mantissa could be negative, due to the shorter */
/* exponent field in IEEE floating-point format, or it could be greater  than */
/* 32  bits,  due  to  the  incredible range of VAX H_floating point numbers. */
/* Since bit shift operations in C must be positive, and there are  no  ANSI- */
/* standard  integer  data  types  larger than 32 bits, special cases must be */
/* used for negative shifts and shifts larger than 32 bits.  Also,  optimized */
/* cases for 0-, 32-, 64-, and 96-bit shifts have been introduced.            */

            shr = 1 - e - 4;
            if ( shr < 0 ) {
               shl    = -shr;
               shr    = 32 - shl;
               *out++ = sign | ( part1 << shl ) | ( part2 >> shr );
               *out++ = ( part2 << shl ) | ( part3 >> shr );
               *out++ = ( part3 << shl ) | ( part4 >> shr );
               *out++ = part4 << shl;
            } else if ( shr == 0 ) {
               *out++ = sign | part1;
               *out++ = part2;
               *out++ = part3;
               *out++ = part4;
            } else if ( shr < 32 ) {
               shl    = 32 - shr;
               *out++ = sign | ( part1 >> shr );
               *out++ = ( part1 << shl ) | ( part2 >> shr );
               *out++ = ( part2 << shl ) | ( part3 >> shr );
               *out++ = ( part3 << shl ) | ( part4 >> shr );
            } else if ( shr == 32 ) {
               *out++ = sign;
               *out++ = part1;
               *out++ = part2;
               *out++ = part3;
            } else if ( shr < 64 ) {
               shl    = 64 - shr;
               shr    = 32 - shl;
               *out++ = sign;
               *out++ = part1 >> shr;
               *out++ = ( part1 << shl ) | ( part2 >> shr );
               *out++ = ( part2 << shl ) | ( part3 >> shr );
            } else if ( shr == 64 ) {
               *out++ = sign;
               *out++ = 0;
               *out++ = part1;
               *out++ = part2;
            } else if ( shr < 96 ) {
               shl    = 96 - shr;
               shr    = 32 - shl;
               *out++ = sign;
               *out++ = 0;
               *out++ = part1 >> shr;
               *out++ = ( part1 << shl ) | ( part2 >> shr );
            } else if ( shr == 96 ) {
               *out++ = sign;
               *out++ = 0;
               *out++ = 0;
               *out++ = part1;
            } else if ( shr <= 112 ) {
               shl    = 128 - shr;
               shr    = 32 - shl;
               *out++ = sign;
               *out++ = 0;
               *out++ = 0;
               *out++ = part1 >> shr;
            } else {
               *out++ = 0;                                       /* Underflow */
               *out++ = 0;
               *out++ = 0;
               *out++ = 0;
            }
         } else if ( e > 1024 ) {
            *out++ = sign | 0x7FF00000;                         /* +-Infinity */
            *out++ = 0;
            *out++ = 0;
            *out++ = 0;
         } else {
            *out++ = sign |                                     /* Normalized */
                     ( ( part1 - ( ( 1 + VAX_BIAS - IEEE_BIAS ) << 16 ) ) <<
                        4 ) | ( part2 >> 28 );
            *out++ = ( part2 << 4 ) | ( part3 >> 28 );
            *out++ = ( part3 << 4 ) | ( part4 >> 28 );
            *out++ = part4 << 4;
         }
      }
   }

}

#undef SIGN_BIT
#undef VAX_EXPONENT
#undef VAX_HIDDEN_BIT
#undef VAX_MANTISSA
#undef VAX_BIAS
#undef IEEE_BIAS

#endif /* #ifdef MAKE_FROM_VAX_H16 */

/**************************************************************** to_vax_i2() */

#ifdef MAKE_TO_VAX_I2

void FORTRAN_LINKAGE to_vax_i2( const void *inbuf, void *outbuf,
                                const int *ntrans ) {

#if LITTLE_ENDIAN

   register const unsigned short *in;   /* Microsoft C: up to 2 register vars */
   register unsigned short *out;        /* Microsoft C: up to 2 register vars */
   int n;


   in  = (const unsigned short *) inbuf;
   out = (unsigned short *) outbuf;

   if ( in != out )
      for ( n = *ntrans; n > 0; n-- )
         *out++ = *in++;

#else

   int n;
   const unsigned char *in;
   unsigned char *out, c1;


   in  = (const unsigned char *) inbuf;
   out = (unsigned char *) outbuf;

   for ( n = *ntrans; n > 0; n-- ) {
      c1     = *in++;
      *out++ = *in++;
      *out++ = c1;
   }

#endif

}

#endif /* #ifdef MAKE_TO_VAX_I2 */

/**************************************************************** to_vax_i4() */

#ifdef MAKE_TO_VAX_I4

void FORTRAN_LINKAGE to_vax_i4( const void *inbuf, void *outbuf,
                                const int *ntrans ) {

#if LITTLE_ENDIAN

   register const unsigned int *in;     /* Microsoft C: up to 2 register vars */
   register unsigned int *out;          /* Microsoft C: up to 2 register vars */
   int n;


   in  = (const unsigned int *) inbuf;
   out = (unsigned int *) outbuf;

   if ( in != out )
      for ( n = *ntrans; n > 0; n-- )
         *out++ = *in++;

#else

   int n;
   const unsigned char *in;
   unsigned char *out, c1, c2;


   in  = (const unsigned char *) inbuf;
   out = (unsigned char *) outbuf;

   for ( n = *ntrans; n > 0; n-- ) {
      c1     = *in++;
      c2     = *in++;
      *out++ = *++in;
      *out++ = *--in;
      in    += 2;
      *out++ = c2;
      *out++ = c1;
   }

#endif

}

#endif /* #ifdef MAKE_TO_VAX_I4 */

/**************************************************************** to_vax_r4() */

#ifdef MAKE_TO_VAX_R4

#define SIGN_BIT        0x80000000
#define IEEE_EXPONENT   0x7F800000
#define IEEE_HIDDEN_BIT 0x00800000
#define IEEE_MANTISSA   0x007FFFFF
#define IEEE_BIAS       127
#define VAX_BIAS        128

void FORTRAN_LINKAGE to_vax_r4( const void *inbuf, void *outbuf,
                                const int *ntrans ) {

   register const unsigned int *in;     /* Microsoft C: up to 2 register vars */
#if LITTLE_ENDIAN
   register unsigned short *out;        /* Microsoft C: up to 2 register vars */
   union { unsigned short i[2]; unsigned int l; } part;
#else
   unsigned char *out;
   union { unsigned char c[4]; unsigned int l; } part;
#endif
   unsigned int part1;
   int n;
   int e;
   unsigned int m;


   in = (const unsigned int *) inbuf;
#if LITTLE_ENDIAN
   out = (unsigned short *) outbuf;
#else
   out = (unsigned char *) outbuf;
#endif

   for ( n = *ntrans; n > 0; n-- ) {
      part1 = *in++;
      if ( ( part1 & ~SIGN_BIT ) == 0 )
         part.l = 0;                                              /* +0 or -0 */
      else if ( ( e = ( part1 & IEEE_EXPONENT ) >> 23 ) == 255 )
         raise( SIGFPE );                                /* +-Infinity, +-NaN */
      else {
         m = part1 & IEEE_MANTISSA;
         if ( e == 0 ) {                                      /* Denormalized */
            m <<= 1;
            while ( ( m & IEEE_HIDDEN_BIT ) == 0 ) {
               m <<= 1;
               e -= 1;                                     /* Adjust exponent */
            }
            m &= IEEE_MANTISSA;
         }
         if ( ( e += 1 + VAX_BIAS - IEEE_BIAS ) <= 0 )
            part.l = 0;                                          /* Underflow */
         else if ( e > 255 )
            raise( SIGFPE );                                      /* Overflow */
         else
            part.l = ( part1 & SIGN_BIT ) | ( e << 23 ) | m;    /* Normalized */
      }
#if LITTLE_ENDIAN
      *out++ = part.i[1];
      *out++ = part.i[0];
#else
      *out++ = part.c[1];
      *out++ = part.c[0];
      *out++ = part.c[3];
      *out++ = part.c[2];
#endif
   }

}

#undef SIGN_BIT
#undef IEEE_EXPONENT
#undef IEEE_HIDDEN_BIT
#undef IEEE_MANTISSA
#undef IEEE_BIAS
#undef VAX_BIAS

#endif /* #ifdef MAKE_TO_VAX_R4 */

/**************************************************************** to_vax_d8() */

#ifdef MAKE_TO_VAX_D8

#define SIGN_BIT        0x80000000
#define IEEE_EXPONENT   0x7FF00000
#define IEEE_HIDDEN_BIT 0x00100000
#define IEEE_MANTISSA   0x000FFFFF
#define IEEE_BIAS       1023
#define VAX_BIAS        128

void FORTRAN_LINKAGE to_vax_d8( const void *inbuf, void *outbuf,
                                const int *ntrans ) {

   register const unsigned int *in;     /* Microsoft C: up to 2 register vars */
#if LITTLE_ENDIAN
   register unsigned short *out;        /* Microsoft C: up to 2 register vars */
   union { unsigned short i[2]; unsigned int l; } part;
#else
   unsigned char *out;
   union { unsigned char c[4]; unsigned int l; } part;
#endif
   unsigned int part1, part2;
   int n;
   int e;
   unsigned int m;


   in = (const unsigned int *) inbuf;
#if LITTLE_ENDIAN
   out = (unsigned short *) outbuf;
#else
   out = (unsigned char *) outbuf;
#endif

   for ( n = *ntrans; n > 0; n-- ) {
      part1 = *in++;
      part2 = *in++;
      if ( ( part1 & ~SIGN_BIT ) == 0 ) {
         part.l = 0;                                              /* +0 or -0 */
         part2 = 0;
      } else if ( ( e = ( part1 & IEEE_EXPONENT ) >> 20 ) == 1023 )
         raise( SIGFPE );                                /* +-Infinity, +-NaN */
      else {
         m = part1 & IEEE_MANTISSA;
         if ( e == 0 ) {                                      /* Denormalized */
            m = ( m << 1 ) | ( part2 >> 31 );
            part2 <<= 1;
            while ( ( m & IEEE_HIDDEN_BIT ) == 0 ) {
               m = ( m << 1 ) | ( part2 >> 31 );
               part2 <<= 1;
               e -= 1;                                     /* Adjust exponent */
            }
            m &= IEEE_MANTISSA;
         }
         if ( ( e += 1 + VAX_BIAS - IEEE_BIAS ) <= 0 ) {
            part.l = 0;                                          /* Underflow */
            part2 = 0;
         } else if ( e > 255 )
            raise( SIGFPE );                                      /* Overflow */
         else {
            part.l = ( part1 & SIGN_BIT ) | ( e << 23 ) |       /* Normalized */
                     ( m << 3 ) | ( part2 >> 29 );
            part2 <<= 3;
         }
      }
#if LITTLE_ENDIAN
      *out++ = part.i[1];
      *out++ = part.i[0];
      part.l = part2;
      *out++ = part.i[1];
      *out++ = part.i[0];
#else
      *out++ = part.c[1];
      *out++ = part.c[0];
      *out++ = part.c[3];
      *out++ = part.c[2];
      part.l = part2;
      *out++ = part.c[1];
      *out++ = part.c[0];
      *out++ = part.c[3];
      *out++ = part.c[2];
#endif
   }

}

#undef SIGN_BIT
#undef IEEE_EXPONENT
#undef IEEE_HIDDEN_BIT
#undef IEEE_MANTISSA
#undef IEEE_BIAS
#undef VAX_BIAS

#endif /* #ifdef MAKE_TO_VAX_D8 */

/**************************************************************** to_vax_g8() */

#ifdef MAKE_TO_VAX_G8

#define SIGN_BIT        0x80000000
#define IEEE_EXPONENT   0x7FF00000
#define IEEE_HIDDEN_BIT 0x00100000
#define IEEE_MANTISSA   0x000FFFFF
#define IEEE_BIAS       1023
#define VAX_BIAS        1024

void FORTRAN_LINKAGE to_vax_g8( const void *inbuf, void *outbuf,
                                const int *ntrans ) {

   register const unsigned int *in;     /* Microsoft C: up to 2 register vars */
#if LITTLE_ENDIAN
   register unsigned short *out;        /* Microsoft C: up to 2 register vars */
   union { unsigned short i[2]; unsigned int l; } part;
#else
   unsigned char *out;
   union { unsigned char c[4]; unsigned int l; } part;
#endif
   unsigned int part1, part2;
   int n;
   int e;
   unsigned int m;


   in = (const unsigned int *) inbuf;
#if LITTLE_ENDIAN
   out = (unsigned short *) outbuf;
#else
   out = (unsigned char *) outbuf;
#endif

   for ( n = *ntrans; n > 0; n-- ) {
      part1 = *in++;
      part2 = *in++;
      if ( ( part1 & ~SIGN_BIT ) == 0 ) {
         part.l = 0;                                              /* +0 or -0 */
         part2 = 0;
      } else if ( ( e = ( part1 & IEEE_EXPONENT ) >> 20 ) == 1023 )
         raise( SIGFPE );                                /* +-Infinity, +-NaN */
      else {
         m = part1 & IEEE_MANTISSA;
         if ( e == 0 ) {                                      /* Denormalized */
            m = ( m << 1 ) | ( part2 >> 31 );
            part2 <<= 1;
            while ( ( m & IEEE_HIDDEN_BIT ) == 0 ) {
               m = ( m << 1 ) | ( part2 >> 31 );
               part2 <<= 1;
               e -= 1;                                     /* Adjust exponent */
            }
            m &= IEEE_MANTISSA;
         }
         if ( ( e += 1 + VAX_BIAS - IEEE_BIAS ) <= 0 ) {
            part.l = 0;                                          /* Underflow */
            part2 = 0;
         } else if ( e > 1023 )
            raise( SIGFPE );                                      /* Overflow */
         else
            part.l = ( part1 & SIGN_BIT ) | ( e << 20 ) | m;    /* Normalized */
      }
#if LITTLE_ENDIAN
      *out++ = part.i[1];
      *out++ = part.i[0];
      part.l = part2;
      *out++ = part.i[1];
      *out++ = part.i[0];
#else
      *out++ = part.c[1];
      *out++ = part.c[0];
      *out++ = part.c[3];
      *out++ = part.c[2];
      part.l = part2;
      *out++ = part.c[1];
      *out++ = part.c[0];
      *out++ = part.c[3];
      *out++ = part.c[2];
#endif
   }

}

#undef SIGN_BIT
#undef IEEE_EXPONENT
#undef IEEE_HIDDEN_BIT
#undef IEEE_MANTISSA
#undef IEEE_BIAS
#undef VAX_BIAS

#endif /* #ifdef MAKE_TO_VAX_G8 */
