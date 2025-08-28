/******************************************************************************
 *                                                                            *
 * bbrhdr.c - Fortran-callable function to read a VFBB file's real headers    *
 *                                                                            *
 *    C format (see note for OpenVMS):                                        *
 *                                                                            *
 *       int bbrhdr( const char *vfbb_file, const int *nfile,                 *
 *                   const int *rhdr_no, float rhdr[128] );                   *
 *                                                                            *
 *       Returns 0 if no errors.  Otherwise, -1: bad calling argument, -2:    *
 *       memory allocation error, -3: rhdr_no does not exist, or the value of *
 *       the C Run-Time Library variable, errno.                              *
 *                                                                            *
 *       Note: On OpenVMS, the first argument is a pointer to a string        *
 *             descriptor, const struct dsc$descriptor_s *vfbb_file.          *
 *                                                                            *
 *    Fortran format:                                                         *
 *                                                                            *
 *       Integer Function BBRHDR( vfbb_file, nfile, rhdr_no, rhdr )           *
 *       Integer    nfile                                                     *
 *       Character  vfbb_file*(nfile)                                         *
 *       Integer    rhdr_no                                                   *
 *       Real       rhdr(128)                                                 *
 *                                                                            *
 *       Returns 0 if no errors.  Otherwise, -1: bad calling argument, -2:    *
 *       memory allocation error, -3: rhdr_no does not exist, or the value of *
 *       the C Run-Time Library variable, errno.                              *
 *                                                                            *
 *    MATLAB format:                                                          *
 *                                                                            *
 *       rhdr = bbrhdr( 'vfbb_file' [ , rhdr_no ] );                          *
 *                                                                            *
 *                                                                            *
 * Author:  Lawrence M. Baker                                                 *
 *          U.S. Geological Survey                                            *
 *          345 Middlefield Road  MS977                                       *
 *          Menlo Park, CA  94025                                             *
 *          baker@usgs.gov                                                    *
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
 * 11-Dec-1991  L. M. Baker      Original version.                            *
 * 24-Mar-1994  L. M. Baker      Convert to MatLab V4.0 External Interface.   *
 * 17-Jul-1997  L. M. Baker      Use Real*8 for local floating-point          *
 *                                  variables.                                *
 *                               Use Open Convert='FDX' to convert F-floats   *
 *                                  in VFBB files to MatLab's IEEE S-floats.  *
 * 24-May-2000  L. M. Baker      Convert to C.                                *
 *                               Convert to MatLab V5.0 API.                  *
 *                               Manually convert VAX F floating-point format *
 *                                  data to IEEE S floating-point format      *
 *                                  (MatLab V5.0 uses IEEE floating-point     *
 *                                  exclusively).                             *
 *  5-Feb-2001  L. M. Baker      Split fopen() FILE variable assignment and   *
 *                                  test for failure into two separate        *
 *                                  statements to avoid Macintosh MrC's       *
 *                                  "possible unintended assignment" warning. *
 *  7-Feb-2001  L. M. Baker      Remove unnecessary break at the end of the   *
 *                                  while ( UsageError ) to avoid Solaris'    *
 *                                  "end-of-loop code not reached" warning.   *
 *                               Change definitions for TRUE and FALSE to     *
 *                                  constant logical expressions ( 0 == 0 and *
 *                                  0 != 0 ).                                 *
 * 23-Feb-2001  L. M. Baker      Merge code from dhead.c to read headers and  *
 *                                  PC format.                                *
 *  6-Mar-2001  L. M. Baker      Create Fortran and C callable versions.      *
 *  7-Mar-2001  L. M. Baker      #include "libvfbb.h".                        *
 *                                                                            *
 ******************************************************************************/

#include "system_type.h"	/* System Type (MAC, TRU64, UNIX, VMS, WIN32) */

#ifndef FORTRAN_LINKAGE
#define FORTRAN_LINKAGE
#endif

#ifdef UPCASE
#define bbrhdr BBRHDR
#endif

#ifdef APPEND_UNDERSCORE
#define bbrhdr bbrhdr##_
#endif

#if ( SYSTEM_TYPE & SYSTEM_TYPE_VMS )
#pragma module bbrhdr		/* To create upper and lower case versions    */
#ifndef VMS
#define VMS
#endif
#endif

/* ANSI-Standard Headers */

#include <errno.h>                                                   /* errno */
#include <stdio.h>          /* fclose(), fopen(), fread(), fseek(), sprintf() */
#ifndef MATLAB_MEX_FILE
#include <stdlib.h>                                         /* NULL, malloc() */
#endif
#include <string.h>                         /* memcpy(), strerror(), strlen() */

/* OpenVMS-Specific Headers */

#ifdef VMS
#include <descrip.h>                                    /* dsc$descriptor_s{} */
#endif

/* Other Headers */

#include "dr100.h"                       /* dr100_int_hdr{}, dr100_real_hdr{} */
#include "libvfbb.h"   /* from_pc_rhdr(), from_vax_i4(), from_vax_ihdr(),     */
                       /* from_vax_r4(), from_vax_rhdr(), is_little_endian(), */
                       /* to_vax_r4(), to_vax_rhdr()                          */

#ifdef MATLAB_MEX_FILE

#include "mex.h"

#ifdef V4_COMPAT                                     /* For MatLab V4 version */

#define mxArray Matrix
#define const_mxArray Matrix
#define mxCreateDoubleMatrix mxCreateFull
#define mxREAL REAL
#define mxIsChar mxIsString

#else                                                /* For MatLab V5 version */

#define const_mxArray const mxArray

#endif

#endif /* #ifdef MATLAB_MEX_FILE */

#ifndef TRUE
#define TRUE  ( 0 == 0 )
#endif
#ifndef FALSE
#define FALSE ( 0 != 0 )
#endif

#if defined( BIG_ENDIAN ) || defined( LITTLE_ENDIAN )
#if defined( BIG_ENDIAN ) && defined( LITTLE_ENDIAN )
#error BIG_ENDIAN and LITTLE_ENDIAN are both #defined
#endif
#ifdef BIG_ENDIAN
#define boolLittleEndian FALSE
#else
#define boolLittleEndian TRUE
#endif
#endif

#ifdef boolLittleEndian
#define is_little_endian() boolLittleEndian
#endif

/* Assume VAX data conversion is required */
#define CONVERT_VAX_DATA
/* No data conversion if VAX/VMS or OpenVMS/Alpha and VAX floating-point */
#if ( SYSTEM_TYPE == SYSTEM_TYPE_VAXVMS ) || \
    ( ( SYSTEM_TYPE == SYSTEM_TYPE_AXPVMS ) && ( __IEEE_FLOAT == 0 ) )
#undef CONVERT_VAX_DATA
#endif

#ifdef MATLAB_MEX_FILE

/************************************************************** mexFunction() */

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const_mxArray *prhs[] )
{

   char   *errmsg;
   char   *errtext;
   double dhdrno;
   int    nfile;

#else /* #ifdef MATLAB_MEX_FILE */

/******************************************************************* bbrhdr() */

int FORTRAN_LINKAGE bbrhdr(
#ifdef VMS
            const struct dsc$descriptor_s *vfbb_file,
#else
            const char *vfbb_file,
#endif
            const int *nfile, const int *rhdr_no, float rhdr[128] ) {

#endif /* #ifdef MATLAB_MEX_FILE */

   struct dr100_int_hdr  intHeader;
   struct dr100_real_hdr realHeader;
   FILE   *fpin;
   char   *file;
   float  rnull;
   const int realHeaderLen = sizeof( realHeader ) / sizeof( float );
#ifndef boolLittleEndian
   int    boolLittleEndian = is_little_endian();
#endif
   int    UsageError;
   int    rhdrno;
   int    iblock;
   int    rblock;
   unsigned int uintPCFormat;
   int    boolPCFormat;
   short  inull;


#ifdef MATLAB_MEX_FILE

   /* Check for proper arguments */

   UsageError = TRUE;
   while ( UsageError ) {

      /* Right-hand side must have 1 or 2 arguments */

      if ( ( nrhs != 1 ) && ( nrhs != 2 ) )
         break;

      /* Argument 1 (file) must be a (scalar) character string */

      if ( ( mxGetM( prhs[0] ) != 1 ) || !mxIsChar( prhs[0] ) )
         break;
      nfile = mxGetN( prhs[0] );
      file = mxCalloc( nfile + 1, sizeof( char ) );
      if ( mxGetString( prhs[0], file, nfile + 1 ) != 0 )
         break;
      if ( nrhs == 2 ) {

         /* Argument 2 (rhdr_no) must be a scalar integer > 0 */

         if ( ( mxGetM( prhs[1] ) != 1 ) || ( mxGetN( prhs[1] ) != 1 ) )
            break;
         dhdrno = *mxGetPr( prhs[1] );
         rhdrno = (int) dhdrno;
         if ( (double) rhdrno != dhdrno )
            break;
         if ( rhdrno <= 0 )
            break;
      } else
         rhdrno = 1;

      /* Left-hand size must have 1 argument (real header matrix) */

      if ( nlhs != 1 )
         break;

      UsageError = FALSE;

   }
   if ( UsageError )
      mexErrMsgTxt( "Usage:  rhdr = bbrhdr( 'vfbb_file' [ , rhdr_no ] );" );

#else /* #ifdef MATLAB_MEX_FILE */

   /* Check for proper arguments */

   file = NULL;
   UsageError = TRUE;
   while ( UsageError ) {
      int n = *nfile;
      if ( n <= 0 )
         break;
#ifdef VMS
      if ( vfbb_file->dsc$w_length < n )
#else
      if ( (int) strlen( vfbb_file ) < n )
#endif
         break;
      file = malloc( n + 1 );
      if ( file == NULL )
         return -2;
#ifdef VMS
      memcpy( file, vfbb_file->dsc$a_pointer, n );
#else
      memcpy( file, vfbb_file, n );
#endif
      file[n] = '\0';
      rhdrno = *rhdr_no;
      if ( rhdrno <= 0 )
         break;

      UsageError = FALSE;

   }
   if ( UsageError ) {
      free( file );
      return -1;
   }

#endif /* #ifdef MATLAB_MEX_FILE */

   /* Open the VFBB file */

   fpin = fopen( file, "rb" );
   if ( !fpin ) {

      /* Open failure */

#ifdef MATLAB_MEX_FILE
      errtext = strerror( errno );
      errmsg = mxCalloc( 22 + nfile + strlen( errtext ) + 1, sizeof( char ) );
      sprintf( errmsg, "bbrhdr: Can't open %s (%s)", file, errtext );
      mexErrMsgTxt( errmsg );
#else
      fprintf( stderr, "bbrhdr: Can't open %s (%s)\n", file,
               strerror( errno ) );
      free( file );
      return errno;
#endif
   }
#ifdef MATLAB_MEX_FILE
   mxFree( file );
#else
   free( file );
#endif

   /* Read and convert (first) Integer header */

   if ( fread( &intHeader, sizeof( intHeader ), 1, fpin ) != 1 ) {
      fclose( fpin );
#ifdef MATLAB_MEX_FILE
      errtext = strerror( errno );
      errmsg = mxCalloc( 42 + strlen( errtext ) + 1, sizeof( char ) );
      sprintf( errmsg, "bbrhdr: Can't read first integer header (%s)",
               errtext );
      mexErrMsgTxt( errmsg );
#else
      fprintf( stderr, "bbrhdr: Can't read first integer header (%s)\n",
               strerror( errno ) );
      return errno;
#endif
   }
#ifdef CONVERT_VAX_DATA
   from_vax_ihdr( &intHeader, &intHeader );
#endif
   inull = intHeader.iundef;

   /* Read and convert (first) Real header */

					/* No. of extra integer header blocks */
   iblock = intHeader.ninthdr == inull ? 0 : intHeader.ninthdr ;
   if ( iblock > 0 )
      if ( fseek( fpin, iblock * sizeof( intHeader ), SEEK_CUR ) != 0 ) {
         fclose( fpin );
#ifdef MATLAB_MEX_FILE
         errtext = strerror( errno );
         errmsg = mxCalloc( 43 + strlen( errtext ) + 1, sizeof( char ) );
         sprintf( errmsg, "bbrhdr: Can't skip extra integer headers (%s)",
                  errtext );
         mexErrMsgTxt( errmsg );
#else
         fprintf( stderr, "bbrhdr: Can't skip extra integer headers (%s)\n",
                  strerror( errno ) );
         return errno;
#endif
      }
   if ( fread( &realHeader, sizeof( realHeader ), 1, fpin ) != 1 ) {
      fclose( fpin );
#ifdef MATLAB_MEX_FILE
      errtext = strerror( errno );
      errmsg = mxCalloc( 39 + strlen( errtext ) + 1, sizeof( char ) );
      sprintf( errmsg, "bbrhdr: Can't read first real header (%s)", errtext );
      mexErrMsgTxt( errmsg );
#else
      fprintf( stderr, "bbrhdr: Can't read first real header (%s)\n",
               strerror( errno ) );
      return errno;
#endif
   }

   uintPCFormat = *((unsigned int *) &realHeader.rundef);
#if !defined( boolLittleEndian ) || !boolLittleEndian
#ifndef boolLittleEndian
   if ( !boolLittleEndian )
#endif
   {
      int n = 1;
      from_vax_i4( &uintPCFormat, &uintPCFormat, &n );
   }
#endif
   boolPCFormat = uintPCFormat == PCGEOS_MAGIC_NUMBER;

#ifdef CONVERT_VAX_DATA
   if ( boolPCFormat ) {
      if ( !boolLittleEndian )
         from_pc_rhdr( &realHeader, &realHeader );
   } else
      from_vax_rhdr( &realHeader, &realHeader );
#else
   if ( boolPCFormat )
      to_vax_rhdr( &realHeader, &realHeader );
#endif
   rnull = realHeader.rundef;

					   /* No. of extra real header blocks */
   rblock = realHeader.nrealhdr == rnull ? 0 : (int) realHeader.nrealhdr ;
   if ( rhdrno-1 > rblock ) {
      fclose( fpin );
#ifdef MATLAB_MEX_FILE
      mexErrMsgTxt( "bbrhdr: No such header" );
#else
      fprintf( stderr, "bbrhdr: No such header\n" );
      return -3;
#endif
   }

   if ( rhdrno > 1 ) {
      if ( rhdrno > 2 )
         if ( fseek( fpin, ( rhdrno - 2 ) * sizeof( realHeader ), SEEK_CUR ) !=
              0 ) {
            fclose( fpin );
#ifdef MATLAB_MEX_FILE
            errtext = strerror( errno );
            errmsg = mxCalloc( 34 + strlen( errtext ) + 1, sizeof( char ) );
            sprintf( errmsg, "bbrhdr: Can't skip real headers (%s)", errtext );
            mexErrMsgTxt( errmsg );
#else
            fprintf( stderr, "bbrhdr: Can't skip real headers (%s)\n",
                     strerror( errno ) );
            return errno;
#endif
         }
      if ( fread( &realHeader, sizeof( realHeader ), 1, fpin ) != 1 ) {
         fclose( fpin );
#ifdef MATLAB_MEX_FILE
         errtext = strerror( errno );
         errmsg = mxCalloc( 33 + strlen( errtext ) + 1, sizeof( char ) );
         sprintf( errmsg, "bbrhdr: Can't read real header (%s)", errtext );
         mexErrMsgTxt( errmsg );
#else
         fprintf( stderr, "bbrhdr: Can't read real header (%s)\n",
                  strerror( errno ) );
         return errno;
#endif
      }
#ifdef CONVERT_VAX_DATA
      if ( boolPCFormat ) {
         if ( !boolLittleEndian )
            from_vax_i4( &realHeader, &realHeader, &realHeaderLen );
      } else
         from_vax_r4( &realHeader, &realHeader, &realHeaderLen );
#else
      if ( boolPCFormat )
         to_vax_r4( &realHeader, &realHeader, &realHeaderLen );
#endif
   }

   fclose( fpin );

#ifdef MATLAB_MEX_FILE

   /* Create a matrix for the real header */

   plhs[0] = mxCreateDoubleMatrix( 1, realHeaderLen, mxREAL );

   /* Copy the real header into the matrix */

   {
      double *matrixP;
      float  *realHeaderP;
      int    n;
      matrixP     = mxGetPr( plhs[0] );
      realHeaderP = (float *) &realHeader;
      for ( n = realHeaderLen; n > 0; --n )
         *matrixP++ = (double) *realHeaderP++;
   }

#else

   /* Copy the real header into rhdr */

   memcpy( rhdr, &realHeader, sizeof( realHeader ) );

   return 0;

#endif

}
