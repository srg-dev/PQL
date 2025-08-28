/******************************************************************************
 *                                                                            *
 * bbdata.c - Fortran-callable function to read a VFBB file's time-series     *
 *                                                                            *
 *    C format (see note for OpenVMS):                                        *
 *                                                                            *
 *       int bbdata( const char *vfbb_file, const int *nfile,                 *
 *                   const int *first_pt, const int *last_pt,                 *
 *                   double unscaled_data[], double *scale_factor,            *
 *                   int *npt );                                              *
 *                                                                            *
 *       Returns 0 if no errors.  Otherwise, -1: bad calling argument, -2:    *
 *       memory allocation error, -3: unsupported VFBB file data type, or the *
 *       value of the C Run-Time Library variable, errno.                     *
 *                                                                            *
 *       Note: On OpenVMS, the first argument is a pointer to a string        *
 *             descriptor, const struct dsc$descriptor_s *vfbb_file.          *
 *                                                                            *
 *    Fortran format:                                                         *
 *                                                                            *
 *       Integer Function BBDATA( vfbb_file, nfile, first_pt, last_pt,        *
 *                                unscaled_data, scale_factor, npt )          *
 *       Integer    nfile                                                     *
 *       Character  vfbb_file*(nfile)                                         *
 *       Integer    first_pt, last_pt                                         *
 *       Double Precision unscaled_data(*)                                    *
 *       Double Precision scale_factor                                        *
 *       Integer    npt                                                       *
 *                                                                            *
 *       Returns 0 if no errors.  Otherwise, -1: bad calling argument, -2:    *
 *       memory allocation error, -3: unsupported VFBB file data type, or the *
 *       value of the C Run-Time Library variable, errno.                     *
 *                                                                            *
 *    MATLAB formats:                                                         *
 *                                                                            *
 *       scaled_data = bbdata( 'vfbb_file' );                                 *
 *       scaled_data = bbdata( 'vfbb_file', first_pt, last_pt );              *
 *       [ unscaled_data, scale ] = bbdata( 'vfbb_file' );                    *
 *       [ unscaled_data, scale ] = bbdata( 'vfbb_file', first_pt, last_pt ); *
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
 *  1-Sep-1992  L. M. Baker      Fixup to sanity checks for gains.            *
 *  3-Sep-1992  L. M. Baker      More fixups to sanity checks for gains       *
 *                                  (courtesy of Lee Steck).                  *
 * 24-Mar-1994  L. M. Baker      Convert to MatLab V4.0 External Interface.   *
 *                               Remove unused arguments from I2DATA, R4DATA. *
 * 31-Mar-1994  L. M. Baker      Gain in dB (rhdr(052)) is 2**(dB/6) for GEOS *
 *                                  playback programs (ihdr(034): RDGEOS=1,   *
 *                                  PCGEOS=4), 10**(dB/20) for all others.    *
 * 17-Sep-1997  L. M. Baker      Support Integer*4 data ( IHDR(4) = DTYPE =   *
 *                                  -4 ).                                     *
 * 14-Jul-1997  L. M. Baker      Return unscaled_data if scale argument       *
 *                                  present.                                  *
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
 *  1-Mar-2001  L. M. Baker      Merge code from dhead.c to read headers and  *
 *                                  PC format.                                *
 *  5-Mar-2001  L. M. Baker      Create Fortran and C callable versions.      *
 *  7-Mar-2001  L. M. Baker      #include "libvfbb.h".                        *
 *                                                                            *
 ******************************************************************************/

#include "system_type.h"	/* System Type (MAC, TRU64, UNIX, VMS, WIN32) */

#ifndef FORTRAN_LINKAGE
#define FORTRAN_LINKAGE
#endif

#ifdef UPCASE
#define bbdata BBDATA
#endif

#ifdef APPEND_UNDERSCORE
#define bbdata bbdata##_
#endif

#if ( SYSTEM_TYPE & SYSTEM_TYPE_VMS )
#pragma module bbdata		/* To create upper and lower case versions    */
#ifndef VMS
#define VMS
#endif
#endif

/* ANSI-Standard Headers */

#include <errno.h>                                                   /* errno */
#include <math.h>                                                    /* pow() */
#include <stdio.h>                 /* fclose(), fopen(), fprintf(), sprintf() */
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
#include "libvfbb.h"      /* from_pc_rhdr(), from_vax_i2(), from_vax_i4(),    */
                          /* from_vax_ihdr(), from_vax_r4(), from_vax_rhdr(), */
                          /* is_little_endian(), to_vax_r4(), to_vax_rhdr()   */

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

/* Function Prototypes */

static int i2data( int block0, double scale, int first, int last,
                   double *matrix );
static int i4data( int block0, double scale, int first, int last,
                   double *matrix );
static int r4data( int block0, double scale, int first, int last,
                   double *matrix );
static int bbread( int recno, void *block );

/* Local Static Data */

#ifndef boolLittleEndian
static int boolLittleEndian;
#endif

static FILE   *fpin;
#ifdef MATLAB_MEX_FILE
static char   *errmsg, *errtext;
#endif

static int    boolPCFormat;

#ifdef MATLAB_MEX_FILE

/************************************************************** mexFunction() */

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const_mxArray *prhs[] )
{

   double dfirst;
   double dlast;
   int    nfile;

#else /* #ifdef MATLAB_MEX_FILE */

/******************************************************************* bbdata() */

int FORTRAN_LINKAGE bbdata(
#ifdef VMS
            const struct dsc$descriptor_s *vfbb_file,
#else
            const char *vfbb_file,
#endif
            const int *nfile, const int *first_pt, const int *last_pt,
            double unscaled_data[], double *scale_factor, int *npt ) {

#endif /* #ifdef MATLAB_MEX_FILE */

   struct dr100_int_hdr  intHeader;
   struct dr100_real_hdr realHeader;
   char   *file;
   double srate;
   double scale;
   double gain;
   float  rnull;
   long   nsize;
   long   np;
   int    UsageError;
   int    first;
   int    last;
   int    format;
   int    playbk;
   int    iblock;
   int    tblock;
   int    rblock;
   int    dblock;
   unsigned int uintPCFormat;
   short  inull;
   short  dtype;
   char   ctype[8];


#ifndef boolLittleEndian
   boolLittleEndian = is_little_endian();
#endif

#ifdef MATLAB_MEX_FILE

   /* Check for proper arguments */

   UsageError = TRUE;
   while ( UsageError ) {

      /* Right-hand side must have 1 or 3 arguments */

      if ( ( nrhs != 1 ) && ( nrhs != 3 ) )
         break;

      /* Argument 1 (vfbb_file) must be a (scalar) character string */

      if ( ( mxGetM( prhs[0] ) != 1 ) || !mxIsChar( prhs[0] ) )
         break;
      nfile = mxGetN( prhs[0] );
      file = mxCalloc( nfile + 1, sizeof( char ) );
      if ( mxGetString( prhs[0], file, nfile + 1 ) != 0 )
         break;
      if ( nrhs == 3 ) {

         /* Argument 2 (first_pt) must be a scalar integer > 0 */

         if ( ( mxGetM( prhs[1] ) != 1 ) || ( mxGetN( prhs[1] ) != 1 ) )
            break;
         dfirst = *mxGetPr( prhs[1] );
         first  = (int) dfirst;
         if ( (double) first != dfirst )
            break;
         if ( first <= 0 )
            break;

         /* Argument 3 (last_pt) must be a scalar integer >= 0 */

         if ( ( mxGetM( prhs[2] ) != 1 ) || ( mxGetN( prhs[2] ) != 1 ) )
            break;
         dlast = *mxGetPr( prhs[2] );
         last  = (int) dlast;
         if ( (double) last != dlast )
            break;
         if ( ( last < 0 ) || ( ( last > 0 ) && ( last < first ) ) )
            break;
      } else {
         first = 1;
         last  = 0;
      }

      /* Left-hand side must have either 1 or 2 arguments */
      /* ((un)scaled_data[,scale])                        */

      if ( ( nlhs != 1 ) && ( nlhs != 2 ) )
         break;

      UsageError = FALSE;

   }
   if ( UsageError )
      mexErrMsgTxt( "Usage:      scaled_data          = "
                    "bbdata( 'vfbb_file' [ , first_pt , last_pt ] );\n"
                    "        [ unscaled_data, scale ]" );

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
      first = *first_pt;
      if ( first <= 0 )
         break;
      last = *last_pt;
      if ( ( last < 0 ) || ( ( last > 0 ) && ( last < first ) ) )
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
      sprintf( errmsg, "bbdata: Can't open %s (%s)", file, errtext );
      mexErrMsgTxt( errmsg );
#else
      fprintf( stderr, "bbdata: Can't open %s (%s)\n", file,
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
      sprintf( errmsg, "bbdata: Can't read first integer header (%s)",
               errtext );
      mexErrMsgTxt( errmsg );
#else
      fprintf( stderr, "bbdata: Can't read first integer header (%s)\n",
               strerror( errno ) );
      return errno;
#endif
   }
#ifdef CONVERT_VAX_DATA
   from_vax_ihdr( &intHeader, &intHeader );
#endif
   inull = intHeader.iundef;

   /* Determine data type (+=Real, -=Integer, ABS()=bytes/sample) */

   dtype = intHeader.datatype == inull ? 0 : intHeader.datatype ;
   switch ( dtype ) {
      case  0:
      case -2: nsize  = 256;
               format = 1;
               break;
      case -4: nsize  = 128;
               format = 2;
               break;
      case  1:
      case  4: nsize  = 128;
               format = 3;
               break;
      default: fclose( fpin );
               sprintf( ctype, "%d", dtype );
#ifdef MATLAB_MEX_FILE
               errmsg = mxCalloc( 32 + strlen( ctype ) + 1, sizeof( char ) );
               sprintf( errmsg, "bbdata: Unsupported data type (%s)", ctype );
               mexErrMsgTxt( errmsg );
#else
               fprintf( stderr, "bbdata: Unsupported data type (%s)\n", ctype );
               return -3;
#endif
               break;
   }

   /* Number of samples (data points) */

   np = ( intHeader.ndatablocks == inull ) || ( intHeader.lastindex == inull ) ?
        intHeader.nsamples :
        ( intHeader.ndatablocks - 1 ) * nsize + intHeader.lastindex ;

   /* Minimize the request with the number available */

   if ( ( last == 0 ) || ( last > np ) )
      last = np;

#ifndef MATLAB_MEX_FILE
   *npt = last - first + 1;
#endif

   /* Playback program identification */

   playbk = intHeader.programname;

   /* Read and convert (first) Real header */

					/* No. of extra integer header blocks */
   iblock = intHeader.ninthdr == inull ? 0 : intHeader.ninthdr ;
   if ( iblock > 0 )
      if ( fseek( fpin, iblock * sizeof( intHeader ), SEEK_CUR ) != 0 ) {
         fclose( fpin );
#ifdef MATLAB_MEX_FILE
         errtext = strerror( errno );
         errmsg = mxCalloc( 43 + strlen( errtext ) + 1, sizeof( char ) );
         sprintf( errmsg, "bbdata: Can't skip extra integer headers (%s)",
                  errtext );
         mexErrMsgTxt( errmsg );
#else
         fprintf( stderr, "bbdata: Can't skip extra integer headers (%s)\n",
                  strerror( errno ) );
         return errno;
#endif
      }
   if ( fread( &realHeader, sizeof( realHeader ), 1, fpin ) != 1 ) {
      fclose( fpin );
#ifdef MATLAB_MEX_FILE
      errtext = strerror( errno );
      errmsg = mxCalloc( 39 + strlen( errtext ) + 1, sizeof( char ) );
      sprintf( errmsg, "bbdata: Can't read first real header (%s)", errtext );
      mexErrMsgTxt( errmsg );
#else
      fprintf( stderr, "bbdata: Can't read first real header (%s)\n",
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

   /* Sample rate */

   srate = realHeader.samplerate;

   /* Scale factor */

   scale = 1.;
				/* Data is not already in ground motion units */
   if ( realHeader.agramhist == rnull )
      if ( ( realHeader.digconst  != rnull ) && ( realHeader.digconst  > 0. ) &&
           ( realHeader.coilconst != rnull ) && ( realHeader.coilconst > 0. ) &&
           ( realHeader.ampgain   != rnull )
         ) {
         /* Gain in dB is 2**(dB/6) for GEOS playback programs */
         /* (RDGEOS=1, PCGEOS=4), 10**(dB/20) for all others.  */
         gain = ( playbk == 1 ) || ( playbk == 4 ) ?
                pow(  2., realHeader.ampgain/ 6. ) :
                pow( 10., realHeader.ampgain/20. ) ;
         scale = 1. / ( realHeader.digconst * realHeader.coilconst * gain );
      }

					   /* No. of extra real header blocks */
   rblock = realHeader.nrealhdr == rnull ? 0 : (int) realHeader.nrealhdr ;
   if ( rblock > 0 )
      if ( fseek( fpin, rblock * sizeof( realHeader ), SEEK_CUR ) != 0 ) {
         fclose( fpin );
#ifdef MATLAB_MEX_FILE
         errtext = strerror( errno );
         errmsg = mxCalloc( 40 + strlen( errtext ) + 1, sizeof( char ) );
         sprintf( errmsg, "bbdata: Can't skip extra real headers (%s)",
                  errtext );
         mexErrMsgTxt( errmsg );
#else
         fprintf( stderr, "bbdata: Can't skip extra real headers (%s)\n",
                  strerror( errno ) );
         return errno;
#endif
      }

   /* Text header no. 1 follows the last real header */

						 /* No. of text header blocks */
   tblock = intHeader.naschdr == inull ? 0 : intHeader.naschdr ;
   if ( tblock > 0 )
      if ( fseek( fpin, tblock * sizeof( intHeader ), SEEK_CUR ) != 0 ) {
         fclose( fpin );
#ifdef MATLAB_MEX_FILE
         errtext = strerror( errno );
         errmsg = mxCalloc( 34 + strlen( errtext ) + 1, sizeof( char ) );
         sprintf( errmsg, "bbdata: Can't skip text headers (%s)", errtext );
         mexErrMsgTxt( errmsg );
#else
         fprintf( stderr, "bbdata: Can't skip text headers (%s)\n",
                  strerror( errno ) );
         return errno;
#endif
      }

#ifdef MATLAB_MEX_FILE

   /* Create a matrix for the time-series */

   plhs[0] = mxCreateDoubleMatrix( 1, last - first + 1, mxREAL );

   /* Copy the scale factor if requested and return unscaled time-series */

   if ( nlhs == 2 ) {
      plhs[1] = mxCreateDoubleMatrix( 1, 1, mxREAL );
      *mxGetPr( plhs[1] ) = scale;
      scale = 1.;
   }

#else /* #ifdef MATLAB_MEX_FILE */

   *scale_factor = scale;
   scale = 1.;

#endif /* #ifdef MATLAB_MEX_FILE */

   /* First data block */

   dblock = 3 + iblock + tblock + rblock;

   /* Read the time-series into the matrix */

#ifdef MATLAB_MEX_FILE

   switch ( format ) {

      case 1:  i2data( dblock, scale, first, last, mxGetPr( plhs[0] ) );
               break;
      case 2:  i4data( dblock, scale, first, last, mxGetPr( plhs[0] ) );
               break;
      default: r4data( dblock, scale, first, last, mxGetPr( plhs[0] ) );
               break;

   }

#else /* #ifdef MATLAB_MEX_FILE */

   switch ( format ) {

      case 1:  if ( i2data( dblock, scale, first, last, unscaled_data ) != 0 )
                  return errno;
               break;
      case 2:  if ( i4data( dblock, scale, first, last, unscaled_data ) != 0 )
                  return errno;
               break;
      default: if ( r4data( dblock, scale, first, last, unscaled_data ) != 0 )
                  return errno;
               break;

   }

#endif /* #ifdef MATLAB_MEX_FILE */

   fclose( fpin );

#ifndef MATLAB_MEX_FILE
   return 0;
#endif

}

/******************************************************************* i2data() */

static int i2data( int dblock, double scale, int first, int last,
                   double *matrix ) {

   short i2data[256];
   int   j, rec, block;
   int   ntrans = { ( sizeof( i2data ) / sizeof( short ) ) };


   /* Read Integer*2 time-series (16-bit 2's complement) */

   rec = 0;
   for ( j = first-1; j < last; j++ ) {
      block = dblock + ( j / ( sizeof( i2data ) / sizeof( short ) ) );
      if ( block != rec ) {
         if ( bbread( block, i2data ) != 0 )
            return errno;

         /* Convert VAX data format to native */

#ifdef CONVERT_VAX_DATA
         from_vax_i2( i2data, i2data, &ntrans );
#endif

         rec = block;
      }
      *matrix++ = scale * i2data[ j % ( sizeof( i2data ) / sizeof( short ) ) ];
   }

   return 0;

}

/******************************************************************* i4data() */

static int i4data( int dblock, double scale, int first, int last,
                   double *matrix ) {

   long  i4data[128];
   int   j, rec, block;
   int   ntrans = { ( sizeof( i4data ) / sizeof( long ) ) };


   /* Read Integer*4 time-series (32-bit 2's complement) */

   rec = 0;
   for ( j = first-1; j < last; j++ ) {
      block = dblock + ( j / ( sizeof( i4data ) / sizeof( long ) ) );
      if ( block != rec ) {
         if ( bbread( block, i4data ) != 0 )
            return errno;

         /* Convert VAX data format to native */

#ifdef CONVERT_VAX_DATA
         from_vax_i4( i4data, i4data, &ntrans );
#endif

         rec = block;
      }
      *matrix++ = scale * i4data[ j % ( sizeof( i4data ) / sizeof( long ) ) ];
   }

   return 0;

}

/******************************************************************* r4data() */

static int r4data( int dblock, double scale, int first, int last,
                   double *matrix ) {

   float r4data[128];
   int   j, rec, block;
   int   ntrans = { ( sizeof( r4data ) / sizeof( float ) ) };


   /* Read Real*4 time-series (32-bit floating-point) */

   rec = 0;
   for ( j = first-1; j < last; j++ ) {
      block = dblock + ( j / ( sizeof( r4data ) / sizeof( float ) ) );
      if ( block != rec ) {
         if ( bbread( block, r4data ) != 0 )
            return errno;

         /* Convert VAX data format to native */

#ifdef CONVERT_VAX_DATA
         if ( boolPCFormat ) {
            if ( !boolLittleEndian )
               from_vax_i4( r4data, r4data, &ntrans );
         } else
            from_vax_r4( r4data, r4data, &ntrans );
#else
         if ( boolPCFormat )
            to_vax_r4( r4data, r4data, &ntrans );
#endif

         rec = block;
      }
      *matrix++ = scale * r4data[ j % ( sizeof( r4data ) / sizeof( float ) ) ];
   }

   return 0;

}

/******************************************************************* bbread() */

static int bbread( int recno, void *block ) {

   int status;


   /* Read the requested block from the VFBB file */

   status = fseek( fpin, ( recno - 1 ) * 512L, SEEK_SET ) == 0;
   if ( status )
      status = fread( block, 512L, 1, fpin ) == 1;
   if ( !status ) {

      /* Read failure */

      fclose( fpin );
#ifdef MATLAB_MEX_FILE
      errtext = strerror( errno );
      errmsg = mxCalloc( 26 + strlen( errtext ) + 1, sizeof( char ) );
      sprintf( errmsg, "bbdata: Can't read data (%s)", errtext );
      mexErrMsgTxt( errmsg );
#else
      fprintf( stderr, "bbdata: Can't read data (%s)\n", strerror( errno ) );
      return errno;
#endif
   }

   return 0;

}
