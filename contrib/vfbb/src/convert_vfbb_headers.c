/******************************************************************************
 *                                                                            *
 * convert_vfbb_headers.c - Convert the VAX-format integer and real headers   *
 *                          in a USGS VFBB blocked-binary (DR100) data file   *
 *                          to/from Unix (IEEE) format using the following    *
 *                          routines from convert_vax_data.c:                 *
 *                                                                            *
 *                           from_vax_i2()  - Byte swap Integer*2             *
 *                           from_vax_r4()  - 32-bit VAX F_floating to        *
 *                                               IEEE S_floating              *
 *                           to_vax_i2()    - Byte swap Integer*2             *
 *                           to_vax_r4()    - 32-bit IEEE S_floating to       *
 *                                               VAX F_floating               *
 *                                                                            *
 * from_vax_ihdr()  - Byte swap VAX-format integers in VFBB integer header;   *
 *                       leave ASCII data alone                               *
 * from_vax_rhdr()  - Convert VAX-format reals in VFBB real header to IEEE    *
 *                       format; leave ASCII data alone                       *
 * from_pc_rhdr()   - Byte swap PC-format (IEEE) reals in VFBB real header;   *
 *                       leave ASCII data alone                               *
 * to_vax_ihdr()    - Byte swap Unix-format integers in VFBB integer header;  *
 *                       leave ASCII data alone                               *
 * to_vax_rhdr()    - Convert IEEE-format reals in VFBB real header to VAX    *
 *                       format; leave ASCII data alone                       *
 *                                                                            *
 * All calls take 2 arguments:                                                *
 *                                                                            *
 *    C       declaration  void name( const void *, void * );                 *
 *                                                                            *
 *            usage        name( in_array, out_array );                       *
 *                                                                            *
 *    Fortran declaration  Subroutine NAME( in_array, out_array )             *
 *                                                                            *
 *            usage        Call NAME( in_array, out_array )                   *
 *                                                                            *
 * The in_array and out_array parameters may refer to the same object.        *
 *                                                                            *
 *                                                                            *
 * See convert_vax_data.c for an explanation of the differences between VAX   *
 * integer and floating-point format and Unix integer and floating-point      *
 * format.                                                                    *
 *                                                                            *
 *                                                                            *
 * If  APPEND_UNDERSCORE  is defined, the entry point names are compiled with *
 * an underscore appended.  This is required so that they can  be  called  by *
 * Fortran  in  cases where the Fortran compiler appends an underscore to ex- *
 * ternally called routines (e.g., Sun Fortran).                              *
 *                                                                            *
 * Normally,  all routines are compiled into a single object module.  To com- *
 * pile a single routine into its own module, define MAKE_routine_name,  sub- *
 * stituting  the upper-case name of the routine for routine_name.  For exam- *
 * ple, MAKE_TO_VAX_IHDR.  (This is useful, for example, to insert  the  rou- *
 * tines  into a library such that a linker may extract only the routines ac- *
 * tually needed by a particular program.)                                    *
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
 * 16-May-2000  L. M. Baker      Original version.                            *
 *  5-Feb-2001  L. M. Baker      Add prototypes for all functions (Matlab's   *
 *                                  MrC command on Macintosh requires them).  *
 * 15-Feb-2001  L. M. Baker      Add from_pc_rhdr() to convert Ed Cranswick's *
 *                                  PC format files (little endian, IEEE      *
 *                                  floats).                                  *
 *  9-Mar-2001  L. M. Baker      #include "convert_vfbb_headers.h".           *
 *                               #include "convert_vax_data.h".               *
 * 29-Apr-2004  L. M. Baker      Copy non-null ASCII data.                    *
 *                                                                            *
 ******************************************************************************/

#include "convert_vfbb_headers.h"	/* UPCASE, APPEND_UNDERSCORE,         */
					/*    FORTRAN_LINKAGE                 */
#include "convert_vax_data.h"		/* from_vax_i2(), from_vax_i4(),      */
					/*    from_vax_r4(), to_vax_i2(),     */
					/*    to_vax_r4()                     */

#if !defined(MAKE_FROM_VAX_IHDR) && !defined(MAKE_FROM_VAX_RHDR) && \
    !defined(MAKE_FROM_PC_RHDR)  && !defined(MAKE_TO_VAX_IHDR)   && \
    !defined(MAKE_TO_VAX_RHDR)

#define MAKE_FROM_VAX_IHDR
#define MAKE_FROM_VAX_RHDR
#define MAKE_FROM_PC_RHDR

#define MAKE_TO_VAX_IHDR
#define MAKE_TO_VAX_RHDR

#endif

#if !defined(__STDC__) && !defined(__cplusplus)  /* const is ANSI C, C++ only */
#define const
#endif

/************************************************************ from_vax_ihdr() */

#ifdef MAKE_FROM_VAX_IHDR

void FORTRAN_LINKAGE from_vax_ihdr( const void *inbuf, void *outbuf ) {

   register const unsigned short *in;   /* Microsoft C: up to 2 register vars */
   register unsigned short *out;        /* Microsoft C: up to 2 register vars */
   int i, n;
   short in_null, out_null;


   in  = (const unsigned short *) inbuf;
   out = (unsigned short *) outbuf;
   in_null = in[2];
   n = 1;
   from_vax_i2( &in_null, &out_null, &n );

   n = 42;                                                /*   1..42   binary */
   from_vax_i2( in, out, &n );
   in += n;
   out += n;

   for ( i = 43; i <= 49; i++ ) {                         /*  43..49   ASCII  */
      *out = ( *in == in_null ) ? out_null : *in ;
      in++;
      out++;
   }

   n = 51;                                                /*  50..100  binary */
   from_vax_i2( in, out, &n );
   in += n;
   out += n;

   for ( i = 101; i <= 200; i++ ) {                       /* 101..200  ASCII  */
      *out = ( *in == in_null ) ? out_null : *in ;
      in++;
      out++;
   }

   n = 9;                                                 /* 201..209  binary */
   from_vax_i2( in, out, &n );
   in += n;
   out += n;

   for ( i = 210; i <= 219; i++ ) {                       /* 210..219  ASCII  */
      *out = ( *in == in_null ) ? out_null : *in ;
      in++;
      out++;
   }

   n = 37;                                                /* 220..256  binary */
   from_vax_i2( in, out, &n );

}

#endif /* #ifdef MAKE_FROM_VAX_IHDR */

/************************************************************ from_vax_rhdr() */

#ifdef MAKE_FROM_VAX_RHDR

void FORTRAN_LINKAGE from_vax_rhdr( const void *inbuf, void *outbuf ) {

   register const float *in;            /* Microsoft C: up to 2 register vars */
   register float *out;                 /* Microsoft C: up to 2 register vars */
   int n;
   float in_null, out_null;


   in  = (const float *) inbuf;
   out = (float *) outbuf;
   in_null = in[1];
   n = 1;
   from_vax_r4( &in_null, &out_null, &n );

   n = 38;                                                /*   1..38   binary */
   from_vax_r4( in, out, &n );
   in += n;
   out += n;

   *out = ( *in == in_null ) ? out_null : *in ;           /*    39     ASCII  */
   in++;
   out++;

   n = 89;                                                /*  40..128  binary */
   from_vax_r4( in, out, &n );

}

#endif /* #ifdef MAKE_FROM_VAX_RHDR */

/************************************************************* from_pc_rhdr() */

#ifdef MAKE_FROM_PC_RHDR

void FORTRAN_LINKAGE from_pc_rhdr( const void *inbuf, void *outbuf ) {

   register const float *in;            /* Microsoft C: up to 2 register vars */
   register float *out;                 /* Microsoft C: up to 2 register vars */
   int n;
   float in_null, out_null;


   in  = (const float *) inbuf;
   out = (float *) outbuf;
   in_null = in[1];
   n = 1;
   from_vax_i4( &in_null, &out_null, &n );

   n = 38;                                                /*   1..38   binary */
   from_vax_i4( in, out, &n );
   in += n;
   out += n;

   *out = ( *in == in_null ) ? out_null : *in ;           /*    39     ASCII  */
   in++;
   out++;

   n = 89;                                                /*  40..128  binary */
   from_vax_i4( in, out, &n );

}

#endif /* #ifdef MAKE_FROM_PC_RHDR */

/************************************************************** to_vax_ihdr() */

#ifdef MAKE_TO_VAX_IHDR

void FORTRAN_LINKAGE to_vax_ihdr( const void *inbuf, void *outbuf ) {

   register const unsigned short *in;   /* Microsoft C: up to 2 register vars */
   register unsigned short *out;        /* Microsoft C: up to 2 register vars */
   int i, n;
   short in_null, out_null;


   in  = (const unsigned short *) inbuf;
   out = (unsigned short *) outbuf;
   in_null = in[2];
   n = 1;
   to_vax_i2( &in_null, &out_null, &n );

   n = 42;                                                /*   1..42   binary */
   to_vax_i2( in, out, &n );
   in += n;
   out += n;

   for ( i = 43; i <= 49; i++ ) {                         /*  43..49   ASCII  */
      *out = ( *in == in_null ) ? out_null : *in ;
      in++;
      out++;
   }

   n = 51;                                                /*  50..100  binary */
   to_vax_i2( in, out, &n );
   in += n;
   out += n;

   for ( i = 101; i <= 200; i++ ) {                       /* 101..200  ASCII  */
      *out = ( *in == in_null ) ? out_null : *in ;
      in++;
      out++;
   }

   n = 9;                                                 /* 201..209  binary */
   to_vax_i2( in, out, &n );
   in += n;
   out += n;

   for ( i = 210; i <= 219; i++ ) {                       /* 210..219  ASCII  */
      *out = ( *in == in_null ) ? out_null : *in ;
      in++;
      out++;
   }

   n = 37;                                                /* 220..256  binary */
   to_vax_i2( in, out, &n );

}

#endif /* #ifdef MAKE_TO_VAX_IHDR */

/************************************************************** to_vax_rhdr() */

#ifdef MAKE_TO_VAX_RHDR

void FORTRAN_LINKAGE to_vax_rhdr( const void *inbuf, void *outbuf ) {

   register const float *in;            /* Microsoft C: up to 2 register vars */
   register float *out;                 /* Microsoft C: up to 2 register vars */
   int n;
   float in_null, out_null;


   in  = (const float *) inbuf;
   out = (float *) outbuf;
   in_null = in[1];
   n = 1;
   to_vax_r4( &in_null, &out_null, &n );

   n = 38;                                                /*   1..38   binary */
   to_vax_r4( in, out, &n );
   in += n;
   out += n;

   *out = ( *in == in_null ) ? out_null : *in ;           /*    39     ASCII  */
   in++;
   out++;

   n = 89;                                                /*  40..128  binary */
   to_vax_r4( in, out, &n );

}

#endif /* #ifdef MAKE_TO_VAX_RHDR */
