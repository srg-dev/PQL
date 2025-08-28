/******************************************************************************
 *                                                                            *
 * DHead  -  Displays  the  contents  of  the  integer  and  real  headers of *
 *           of VFBB/DR100 format data files.                                 *
 *                                                                            *
 * MacIntosh MPW/Unix/Win32 command line format:                              *
 *                                                                            *
 *    $ dhead [ ? | -? | -h | -help ]  or                                     *
 *    $ dhead { -v | -version }        or                                     *
 *    $ dhead DR100-file [ { -o | -output } listing-file[.lis] ]              *
 *                                                                            *
 * OpenVMS command line format:                                               *
 *                                                                            *
 *    $ DHEAD /HELP    or                                                     *
 *    $ DHEAD /VERSION or                                                     *
 *    $ DHEAD DR100-file [ /OUTPUT=listing-file[.lis] ]                       *
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
 * 28-Feb-2001  L. M. Baker      Original C version (from DHead.for).         *
 *  7-Mar-2001  L. M. Baker      #include "libvfbb.h".                        *
 * 28-Sep-2004  L. M. Baker      Add support for Mac OS X.                    *
 * 30-Nov-2004  L. M. Baker      Add support for Linux.                       *
 *                                                                            *
 ******************************************************************************/

#include "system_type.h"	/* System Type (MAC, TRU64, UNIX, VMS, WIN32) */

#if ( SYSTEM_TYPE & SYSTEM_TYPE_MACOS ) && !defined( MAC )
#define MAC
#endif

#if ( SYSTEM_TYPE & SYSTEM_TYPE_MACOSX ) && !defined( UNIX )
#define UNIX
#endif
 
#if ( SYSTEM_TYPE == SYSTEM_TYPE_LINUX ) && !defined( LINUX )
#define LINUX
#endif

#if ( SYSTEM_TYPE == SYSTEM_TYPE_TRU64 ) && !defined( TRU64 )
#define TRU64
#endif

#if ( SYSTEM_TYPE & SYSTEM_TYPE_UNIX ) && !defined( UNIX )
#define UNIX
#endif

#if ( SYSTEM_TYPE & SYSTEM_TYPE_VMS ) && !defined( VMS )
#define VMS
#endif

#if ( SYSTEM_TYPE & SYSTEM_TYPE_WIN32 ) && !defined( WIN32 )
#define WIN32
#endif

/* ANSI-Standard Headers */

#include <errno.h>		/* errno */
#include <limits.h>		/* PATH_MAX */
#include <stdio.h>		/* FILENAME_MAX, SEEK_CUR, fclose(),          */
				/*    fgetname(), fileno(), fread(),          */
				/*    freopen(), fseek(), perror(), printf(), */
				/*    putchar(), puts()                       */
#include <stdlib.h>		/* abs(), EXIT_SUCCESS, free(), malloc(), */
				/*    realpath()                          */
#include <string.h>		/* strcat(), strcpy(), strlen(), strncpy(), */
				/*    strrchr()                             */
#include <time.h>		/* time_t, tm{}, localtime(), strftime(), */
				/*    time()                              */

/* MacIntosh-Specific Headers */

#ifdef MAC
#include <IOCtl.h>		/* FIOFNAME, ioctl() */
#endif

/* OpenVMS-Specific Headers */

#ifdef VMS
#include <ssdef.h>		/* SS$_NORMAL */
#include <stsdef.h>		/* STS$M_SUCCESS */
#endif

/* Windows-Specific Headers */

#ifdef WIN32
#include <windows.h>		/* GetFullPathName() */
#endif

/* Other Headers */

#include "dr100.h"		/* dr100_int_hdr{}, dr100_real_hdr{} */
#include "libvfbb.h"		/* from_pc_rhdr(), from_vax_i4(),       */
				/*    from_vax_ihdr(), from_vax_rhdr(), */
				/*    is_little_endian(), to_vax_rhdr() */

/* Local Definitions */

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

#ifdef VMS
#ifdef EXIT_SUCCESS
#undef EXIT_SUCCESS
#endif
#define EXIT_SUCCESS SS$_NORMAL
#define IS_CV_OK( cv ) ( cv & STS$M_SUCCESS )
#endif
#ifndef IS_CV_OK
#define IS_CV_OK( cv ) cv
#endif

/* MacIntosh's ioctl( ... FIOFNAME, ... ) requires a 255 character buffer */
/* OpenVMS' fgetname() requires a 256 character buffer */
#define FULLNAME_MAX 256
#ifdef FILENAME_MAX
#if FILENAME_MAX > FULLNAME_MAX
#undef FULLNAME_MAX
#define FULLNAME_MAX FILENAME_MAX
#endif
#endif
/* Tru64 UNIX's realpath() requires a PATH_MAX character buffer */
#ifdef PATH_MAX
#if PATH_MAX > FULLNAME_MAX
#undef FULLNAME_MAX
#define FULLNAME_MAX PATH_MAX
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

int parse_command_line( int argc, char *argv[] );
FILE *freopen_with_suffix( const char *file_spec, const char *a_mode,
                           FILE *file_ptr, const char *default_suffix );
void print_banner();
void print_fullname();
#if defined( UNIX ) && !defined( LINUX ) && !defined( TRU64 )
void getpath( const char *name, char *path, size_t pathmax );
#endif
int dhead();

/* Global Data */

int boolCLIHelp;
int boolCLIVersion;
char *strCLIDR100File;
int boolCLIOutput;
char *strCLIOutput;


/********************************************************************* main() */

/* Local Static Data */

#ifndef boolLittleEndian
static int boolLittleEndian;
#endif

static const char strBANNER[] = "VFBB/DR100 File Header Dump Utility V2.0";
static const char strUSAGE[] =
#ifdef VMS
   "Usage:  $ DHead /Help\n"
   "   or:  $ DHead /Version\n"
   "   or:  $ DHead DR100-file [ /Output=listing-file[.lis] ]";
#else
   "Usage:  % dhead [ ? | -? | -h | -help ]\n"
   "   or:  % dhead { -v | -version }\n"
   "   or:  % dhead DR100-file [ { -o | -output } listing-file[.lis] ]";
#endif


int main( int argc, char *argv[] ) {

   int      status;


#ifndef boolLittleEndian
   boolLittleEndian = is_little_endian();
#endif

   status = parse_command_line( argc, argv );
   if ( !IS_CV_OK( status ) )
      return status;

   if ( boolCLIHelp || boolCLIVersion ) {

      /* If /Help or /Version was specified, print version no. */

      puts( strBANNER );

      /* If /Help was specified, print the help message */

      if ( boolCLIHelp )
         puts( strUSAGE );

      /* If /Version was specified, print system type and byte order */

      if ( boolCLIVersion )
         printf( "%s, %s Endian, %s Floating-Point\n", SYSTEM_TYPE_NAME,
                 boolLittleEndian ? "Little" : "Big",
#ifdef CONVERT_VAX_DATA
                 "IEEE"
#else
                 "VAX"
#endif
               );

      /* In either case, exit immediately */

      return EXIT_SUCCESS;

   }

   /* Open the DR100 file (stdin) */

   if ( freopen( strCLIDR100File, "rb", stdin ) == NULL ) {
      perror( strCLIDR100File );
      return errno;
   }

   /* Open the output file (stdout; default file type is .lis) */

   if ( boolCLIOutput )

#ifdef VMS
      if ( freopen( strCLIOutput, "w", stdout, "dna=.lis" ) == NULL ) {
#else
      if ( freopen_with_suffix( strCLIOutput, "w", stdout, "lis" ) == NULL ) {
#endif
         perror( strCLIOutput );
         return errno;
      }

   /* Print banner with today's date and time */

   print_banner();

   /* Print full name of DR100 file */

   print_fullname();

   /* Format the DR100 file headers */

   status = dhead();

   fclose( stdin );
   fclose( stdout );

   return status;

}


/****************************************************** freopen_with_suffix() */

#ifndef VMS

FILE *freopen_with_suffix( const char *file_spec, const char *a_mode,
                           FILE *file_ptr, const char *default_suffix ) {

      char       *strOutput;
      const char *cp;
      FILE       *fp = NULL;

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#endif
#ifdef MAC
#define PATH_SEPARATOR ':'
#endif
#ifndef PATH_SEPARATOR
#define PATH_SEPARATOR '/'
#endif

      cp = strrchr( file_spec, PATH_SEPARATOR );
      if ( cp == NULL )
         cp = file_spec;
      if ( strrchr( cp, '.' ) == NULL ) {
         strOutput = (char *) malloc( strlen( file_spec ) +
                                      1 + strlen( default_suffix ) + 1 );
         if ( strOutput != NULL ) {
            strcpy( strOutput, file_spec );
            strcat( strOutput, "." );
            strcat( strOutput, default_suffix );
            fp = freopen( strOutput, a_mode, file_ptr );
            free( strOutput );
         }
      } else
         fp = freopen( file_spec, a_mode, file_ptr );

      return fp;

}

#endif /* #ifndef VMS */


/************************************************************* print_banner() */

void print_banner() {

   time_t   timeUTC;
   struct tm *tmLocalTimeP;
   char     strLocalTime[20];


   time( &timeUTC );
   tmLocalTimeP = localtime( &timeUTC );
   strftime( strLocalTime, 20, "%m/%d/%Y %H:%M:%S", tmLocalTimeP );
   printf( "%-61.61s%19.19s\n", strBANNER, strLocalTime );

}


/*********************************************************** print_fullname() */

void print_fullname() {

   char     strFileName[FULLNAME_MAX];
#ifdef WIN32
   LPTSTR   lpFilePart;
#endif


#ifdef MAC
   /* Use only for a program running as an MPW tool */
   if ( ioctl( fileno( stdin ), FIOFNAME, (long *) strFileName ) < 0 )
      puts( strCLIDR100File );
   else
   /* Fall throuth to puts( strFileName ); */
#endif
#ifdef UNIX
#if defined( LINUX ) || defined( TRU64 )
   realpath( strCLIDR100File, strFileName );
#else
   getpath( strCLIDR100File, strFileName, sizeof( strFileName ) );
#endif
#endif
#ifdef VMS
   fgetname( stdin, strFileName );
#endif
#ifdef WIN32
   GetFullPathName( strCLIDR100File, sizeof( strFileName ), strFileName,
                    &lpFilePart );
#endif

   puts( strFileName );
   putchar( '\n' );

}


/******************************************************************** dhead() */

#define IHDR_VALUE( name, format, missing ) \
{ \
   if ( intHeader.name == inull ) \
      strcpy( name, missing ); \
   else \
      sprintf( name, format, intHeader.name ); \
}
#define RHDR_VALUE( name, format, missing ) \
{ \
   if ( realHeader.name == rnull ) \
      strcpy( name, missing ); \
   else \
      sprintf( name, format, realHeader.name ); \
}
#define Q2 &strQ4[2]
#define Q3 &strQ4[1]
#define Q4 strQ4
#define NULL4 strNULL4

static float rnull;
static short int inull;
static const char strQ4[] = "????";
static const char strNULL4[] = "null";

#define NEWLINE putchar( '\n' )
#define NEXT    putchar( ' ' ); putchar( ' ' )

int dhead() {

   int boolPCFormat;
   unsigned int uintPCFormat;
   short int iblock;
   struct dr100_int_hdr  intHeader;
   struct dr100_real_hdr realHeader;


   /* Read and convert Integer header */

   if ( fread( &intHeader, sizeof( intHeader ), 1, stdin ) != 1 ) {
      perror( "Read first integer header" );
      return errno;
   }
#ifdef CONVERT_VAX_DATA
   from_vax_ihdr( &intHeader, &intHeader );
#endif
   inull = intHeader.iundef;

   /* Read and convert Real header */

					/* No. of extra integer header blocks */
   iblock = intHeader.ninthdr == inull ? 0 : intHeader.ninthdr ;
   if ( iblock > 0 )
      if ( fseek( stdin, iblock * sizeof( intHeader ), SEEK_CUR ) != 0 ) {
         perror( "Skip extra integer headers" );
         return errno;
      }
   if ( fread( &realHeader, sizeof( realHeader ), 1, stdin ) != 1 ) {
      perror( "Read first real header" );
      return errno;
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

   /* Start time:    YYYY DDD-HH:MM:SS.MMMUUU */

   {
      char year[5], day[4], hour[3], minute[3], second[3], msec[4], usec[4];
      IHDR_VALUE( year,   "%4d",  Q4 );
      IHDR_VALUE( day,    "%3d",  Q3 );
      IHDR_VALUE( hour,   "%2d",  Q3 );
      IHDR_VALUE( minute, "%02d", Q2 );
      IHDR_VALUE( second, "%02d", Q2 );
      IHDR_VALUE( msec,   "%03d", Q3 );
      IHDR_VALUE( usec,   "%03d", Q3 );
      printf( "Start time:    %s %s-%s:%s:%s.%s%s",
              year, day, hour, minute, second, msec, usec );
   }
   NEXT;

   /* Component no./No. components: NNNN/NNNN */

   {
      char componentno[14], ntotcomp[7];
      IHDR_VALUE( componentno, "%d", NULL4 );
      IHDR_VALUE( ntotcomp,    "%d", NULL4 );
      strcat( componentno, "/" );
      strcat( componentno, ntotcomp );
      printf( "Component no./No. components: %9.9s", componentno );
   }
   NEWLINE;

   /* No. samples:                     NNNNNN */

   {
      char nsamples[15];
      IHDR_VALUE( nsamples, "%d", NULL4 );
      if ( ( intHeader.nsamples == inull ) &&
           ( ( intHeader.ndatablocks != inull ) &&
             ( intHeader.lastindex != inull ) ) )
         sprintf( nsamples, "(%d)",
                  ( intHeader.ndatablocks - 1 ) * 256 + intHeader.lastindex );
      printf( "No. samples:             %14.14s", nsamples );
   }
   NEXT;

   /* This channel no.:                NNNNNN */

   {
      char nactchan[7];
      IHDR_VALUE( nactchan, "%d", NULL4 );
      printf( "This channel no.:                %6.6s", nactchan );
   }
   NEWLINE;

   /* Block count/index:        NNNNNN/NNNNNN */

   {
      char ndatablocks[14], lastindex[7];
      IHDR_VALUE( ndatablocks, "%d", NULL4 );
      IHDR_VALUE( lastindex,   "%d", NULL4 );
      strcat( ndatablocks, "/" );
      strcat( ndatablocks, lastindex );
      printf( "Block count/index:        %13.13s", ndatablocks );
   }
   NEXT;

   /* First/total channels:     NNNNNN/NNNNNN */

   {
      char nfirchan[14], ntotchan[7];
      IHDR_VALUE( nfirchan, "%d", NULL4 );
      IHDR_VALUE( ntotchan, "%d", NULL4 );
      strcat( nfirchan, "/" );
      strcat( nfirchan, ntotchan );
      printf( "First/total channels:     %13.13s", nfirchan );
   }
   NEWLINE;

   /* Sample rate:             NNNNNNNNNNNN/s */

   {
      char samplerate[15];
      RHDR_VALUE( samplerate, "%.5g/s", NULL4 );
      printf( "Sample rate:             %14.14s", samplerate );
   }
   NEXT;

   /* Experiment/event no.:     NNNNNN/NNNNNN */

   {
      char ntape[14], eventno[7];
      IHDR_VALUE( ntape,    "%d", NULL4 );
      IHDR_VALUE( eventno,  "%d", NULL4 );
      strcat( ntape, "/" );
      strcat( ntape, eventno );
      printf( "Experiment/event no.:     %13.13s", ntape );
   }
   NEWLINE;

   /* Sample lag:              NNNNNNNNNNNN s */

   {
      char samplelag[15];
      RHDR_VALUE( samplelag,  "%.5g s", NULL4 );
      printf( "Sample lag:              %14.14s", samplelag );
   }
   NEXT;

   /* Location number:                 NNNNNN */

   {
      char nlocnum[7];
      IHDR_VALUE( nlocnum,  "%d", NULL4 );
      printf( "Location number:                 %6.6s", nlocnum );
   }
   NEWLINE;

   /* Motion:                  AAAAAAAAAAAAAA */

   {
      char motion[15];
      switch ( intHeader.motion ) {
         case 1:  strcpy( motion, "Acceleration" );
                  break;
         case 2:  strcpy( motion, "Velocity" );
                  break;
         case 3:  strcpy( motion, "Displacement" );
                  break;
         case 50: strcpy( motion, "Strain" );
                  break;
         default: IHDR_VALUE( motion, "%d", NULL4 );
                  break;
      }
      printf( "Motion:                  %14.14s",
              motion );
   }
   NEXT;

   /* Instrument type:             AAAAAAAAAA */

   {
      char recordertype[11];
      switch ( intHeader.recordertype ) {
         case 1:  strcpy( recordertype, "GEOS" );
                  break;
         case 2:  strcpy( recordertype, "DR100" );
                  break;
         case 3:  strcpy( recordertype, "RefTek" );
                  break;
         case 4:  strcpy( recordertype, "Tustin" );
                  break;
         case 5:  strcpy( recordertype, "Synthetic" );
                  break;
         case 6:  strcpy( recordertype, "SSR-1" );
                  break;
         case 7:  strcpy( recordertype, "PDAS-100" );
                  break;
         case 8:  strcpy( recordertype, "K2" );
                  break;
         default: IHDR_VALUE( recordertype, "%d", NULL4 );
                  break;
      }
      printf( "Instrument type:             %10.10s", recordertype );
   }
   NEWLINE;

   /* Transducer type:                   AAAA */

   {
      char transtype[5];
      char *cp;
      if ( realHeader.transtype[0] == rnull )
         strcpy( transtype, NULL4 );
      else
         strncpy( transtype, (char *) realHeader.transtype, 4 );
         transtype[4] = '\0';
         for ( cp = &transtype[3];
               ( cp >= &transtype[0] ) && ( *cp == ' ' );
               *cp-- = '\0' ) {}
      printf( "Transducer type:                   %4.4s", transtype );
   }
   NEXT;

   /* Recorder serial no.:             NNNNNN */

   {
      char serialno[7];
      IHDR_VALUE( serialno, "%d", NULL4 );
      printf( "Recorder serial no.:             %6.6s", serialno );
   }
   NEWLINE;

   /* Orientation--V/H (deg):   NNNNNN/NNNNNN */

   {
      char vertical[14], horizontal[7];
      IHDR_VALUE( vertical,    "%d", NULL4 );
      IHDR_VALUE( horizontal,  "%d", NULL4 );
      strcat( vertical, "/" );
      strcat( vertical, horizontal );
      printf( "Orientation--V/H (deg):   %13.13s", vertical );
   }
   NEXT;

   /* Version--instr/softw:     NNNNNN/NNNNNN */

   {
      char recorderversion[14], recordersubversion[7];
      IHDR_VALUE( recorderversion,    "%d", NULL4 );
      IHDR_VALUE( recordersubversion, "%d", NULL4 );
      strcat( recorderversion, "/" );
      strcat( recorderversion, recordersubversion );
      printf( "Version--instr/softw:     %13.13s", recorderversion );
   }
   NEWLINE;

   /* Sens model/ser. no.:  AAAAAAAAAA/NNNNNN */

   {
      char sensormodel[22], sensorno[7];
      char *cp;
      if ( intHeader.sensormodel[0] == inull )
         strcpy( sensormodel , NULL4 );
      else {
         strncpy( sensormodel , (char *) intHeader.sensormodel, 14 );
         sensormodel[14] = '\0';
         for ( cp = &sensormodel[13];
               ( cp >= &sensormodel[0] ) && ( *cp == ' ' );
               *cp-- = '\0' ) {}
      }
      IHDR_VALUE( sensorno,           "%d", NULL4 );
      strcat( sensormodel, "/" );
      strcat( sensormodel, sensorno );
      printf( "Sens model/ser. no.:  %17.17s", sensormodel );
   }
   NEXT;

   /* Event type:              AAAAAAAAAAAAAA */

   {
      char eventtype[15];
      switch ( intHeader.eventtype ) {
         case 0:  strcpy( eventtype, "Continuous" );
                  break;
         case 1:  strcpy( eventtype, "Trigger" );
                  break;
         case 2:  strcpy( eventtype, "Preset" );
                  break;
         case 3:  strcpy( eventtype, "Calibration" );
                  break;
         default: IHDR_VALUE( eventtype, "%d", NULL4 );
                  break;
      }
      printf( "Event type:              %14.14s", eventtype );
   }
   NEWLINE;

   /* Trans natural freq:      NNNNNNNNNNN Hz */

   {
      char resonance[15];
      RHDR_VALUE( resonance, "%.4g Hz", NULL4 );
      printf( "Trans natural freq:      %14.14s", resonance );
   }
   NEXT;

   /* Trigger algorithm:       AAAAAAAAAAAAAA */

   {
      char ntrig[15];
      switch ( intHeader.ntrig ) {
         case 1:  strcpy( ntrig, "STA/LTA" );
                  break;
         case 2:  strcpy( ntrig, "Teleseismic" );
                  break;
         default: IHDR_VALUE( ntrig, "%d", NULL4 );
                  break;
      }
      printf( "Trigger algorithm:       %14.14s", ntrig );
   }
   NEWLINE;

   /* Trans damping coef:      NNNNNNNNNNNNNN */

   {
      char damping[15];
      RHDR_VALUE( damping,   "%.7g",    NULL4 );
      printf( "Trans damping coef:      %14.14s", damping );
   }
   NEXT;

   /* Trigger--STA/LTA:        NNNNNNN/NNNNNN */

   {
      char nsta[15], nlta[7];
      if ( intHeader.nsta == inull )
         strcpy( nsta, NULL4 );
      else {
         sprintf( nsta, "%.1f", intHeader.nsta / 10. );
      }
      IHDR_VALUE( nlta, "%d", NULL4 );
      strcat( nsta, "/" );
      strcat( nsta, nlta );
      printf( "Trigger--STA/LTA:        %14.14s", nsta );
   }
   NEWLINE;

   /* Coil constant:           NNNNNNNNNNNNNN */

   {
      char coilconst[15];
      RHDR_VALUE( coilconst, "%.7g", NULL4 );
      printf( "Coil constant:           %14.14s", coilconst );
   }
   NEXT;

   /* Trigger--chan/ratio:      NNNNNN/NNNNNN */

   {
      char ntrcomp[14], nrat[7];
      IHDR_VALUE( ntrcomp,    "%d", NULL4 );
      IHDR_VALUE( nrat,       "%d", NULL4 );
      strcat( ntrcomp, "/" );
      strcat( ntrcomp, nrat );
      printf( "Trigger--chan/ratio:      %13.13s", ntrcomp );
   }
   NEWLINE;

   /* Anti-alias corner freq:  NNNNNNNNNNN Hz */

   {
      char aacorner[15];
      RHDR_VALUE( aacorner, "%.4g Hz", NULL4 );
      printf( "Anti-alias corner freq:  %14.14s", aacorner );
   }
   NEXT;

   /* Pre-event/post trig:     NNNNNNN/NNNNNN */

   {
      char npreevent[15], npostevent[7];
      if ( intHeader.npreevent == inull )
         strcpy( npreevent, NULL4 );
      else {
         sprintf( npreevent, "%.1f", intHeader.npreevent / 10. );
      }
      IHDR_VALUE( npostevent, "%d", NULL4 );
      strcat( npreevent, "/" );
      strcat( npreevent, npostevent );
      printf( "Pre-event/post trig:     %14.14s", npreevent );
   }
   NEWLINE;

   /* Poles of AAF:            NNNNNNNNNNNNNN */

   {
      char aapoles[15];
      RHDR_VALUE( aapoles,   "%.7g", NULL4 );
      printf( "Poles of AAF:            %14.14s", aapoles );
   }
   NEXT;

   /* Clock standard:          AAAAAAAAAAAAAA */

   {
      char clocktype[15];
      switch ( intHeader.clocktype ) {
         case 0:  strcpy( clocktype, "None" );
                  break;
         case 1:  strcpy( clocktype, "WWVB" );
                  break;
         case 2:  strcpy( clocktype, "Master clock" );
                  break;
         case 3:  strcpy( clocktype, "Manual" );
                  break;
         case 4:  strcpy( clocktype, "OMEGA" );
                  break;
         case 5:  strcpy( clocktype, "GOES" );
                  break;
         case 6:  strcpy( clocktype, "GPS" );
                  break;
         default: IHDR_VALUE( clocktype, "%d", NULL4 );
                  break;
      }
      printf( "Clock standard:          %14.14s", clocktype );
   }
   NEWLINE;

   /* Amplifier gain:          NNNNNNNNNNN dB */

   {
      char ampgain[15];
      RHDR_VALUE( ampgain,   "%.4g dB", NULL4 );
      printf( "Amplifier gain:          %14.14s", ampgain );
   }
   NEXT;

   /* Time since clock corr.:  NNNNNNNNNNNN s */

   {
      char clocksec[15];
      RHDR_VALUE( clocksec, "%.5g s", NULL4 );
      printf( "Time since clock corr.:  %14.14s", clocksec );
   }
   NEWLINE;

   /* Digitizing constant:     NNNNNNNNNNNNNN */

   {
      char digconst[15];
      RHDR_VALUE( digconst, "%.7g",    NULL4 );
      printf( "Digitizing constant:     %14.14s", digconst );
   }
   NEXT;

   /* Clock correction:        NNNNNNNNNNNN s */

   {
      char clockcorr[15];
      RHDR_VALUE( clockcorr, "%.5g s",  NULL4 );
      printf( "Clock correction:        %14.14s", clockcorr );
   }
   NEWLINE;

   /* Playback pgm/Data fmt:AAAAAA NNN.NN/AAA */

   {
      char programname[18], programversion[8], programsubversion[8];
      if ( intHeader.programname == inull )
         strcpy( programname, "null" );
      else {
         switch ( intHeader.programname ) {
            case 1:  strcpy( programname, "RDGEOS" );
                     break;
            case 2:  strcpy( programname, "AFTAPE" );
                     break;
            case 3:  strcpy( programname, "ANZA"   );
                     break;
            case 4:  strcpy( programname, "PCGEOS" );
                     break;
            case 5:  strcpy( programname, "CRTAPE" );
                     break;
            case 6:  strcpy( programname, "RDSSR"  );
                     break;
            case 7:  strcpy( programname, "RDPDAS" );
                     break;
            default: IHDR_VALUE( programname, "%d", NULL4 );
                     break;
         }
         if ( intHeader.programversion != inull ) {
            sprintf( programversion, " %d", intHeader.programversion );
            strcat( programname, programversion );
            if ( intHeader.programsubversion != inull ) {
               sprintf( programsubversion, ".%d", intHeader.programsubversion );
               if ( 17 - strlen( programname ) - strlen( programsubversion ) -
                    ( boolPCFormat ? 3 : 4 ) > 0 )
                  strcat( programname, programsubversion );
            }
         }
      }
      strcat( programname, boolPCFormat ? "/PC" : "/VAX" );
      printf( "Playback pgm/Data fmt:%17.17s", programname );
   }
   NEXT;

   /* Battery charge:          NNNNNNNNNNNN V */

   {
      char voltage[15];
      RHDR_VALUE( voltage, "%.5g V", NULL4 );
      printf( "Battery charge:          %14.14s", voltage );
   }
   NEWLINE;

   /* Latitude (degrees):      NNNNNNNNNNNNNN */

   {
      char latitude[15];
      RHDR_VALUE( latitude,  "%.7g", NULL4 );
      printf( "Latitude (degrees):      %14.14s", latitude );
   }
   NEXT;

   /* X offset (north):        NNNNNNNNNNNN m */

   {
      char xoffset[15];
      RHDR_VALUE( xoffset, "%.5g m", NULL4 );
      printf( "X offset (north):        %14.14s", xoffset );
   }
   NEWLINE;

   /* Longitude (degrees):     NNNNNNNNNNNNNN */

   {
      char longitude[15];
      RHDR_VALUE( longitude, "%.7g", NULL4 );
      printf( "Longitude (degrees):     %14.14s", longitude );
   }
   NEXT;

   /* Y offset (east):         NNNNNNNNNNNN m */

   {
      char yoffset[15];
      RHDR_VALUE( yoffset, "%.5g m", NULL4 );
      printf( "Y offset (east):         %14.14s", yoffset );
   }
   NEWLINE;

   /* Elevation:               NNNNNNNNNNNN m */

   {
      char elevation[15];
      RHDR_VALUE( elevation, "%.5g m", NULL4 );
      printf( "Elevation:               %14.14s", elevation );
   }
   NEXT;

   /* Z offset (down):         NNNNNNNNNNNN m */

   {
      char zoffset[15];
      RHDR_VALUE( zoffset, "%.5g m", NULL4 );
      printf( "Z offset (down):         %14.14s", zoffset );
   }
   NEWLINE;

   /* Note: the Poles and Zeros lists are not confined to a single column */

   {
      int i, l, n;
      char line[80], z[33], zr[15], zi[15];

      /* Poles: */

      strcpy( line, "Poles:" );
      n = strlen( line );
      if ( intHeader.npoles == inull )
         strcat( line, " null" );
      else if ( intHeader.npoles == 0 )
         strcat( line, " none" );
      else
         for ( i = 0; i < intHeader.npoles; ++i ) {
            sprintf( zr, "%.7g", realHeader.poles[2*i  ] );
            sprintf( zi, "%.7g", realHeader.poles[2*i+1] );
            sprintf( z, " (%.14s,%.14s)", zr, zi );
            l = strlen( z );
            if ( n + l > 80 ) {
               puts( line );
               strcpy( line, "      " );
               n = strlen( line );
            }
            strcat( line, z );
         }
      puts( line );

      /* Zeros: */

      strcpy( line, "Zeros:" );
      if ( intHeader.nzeros == inull )
         strcat( line, " null" );
      else if ( intHeader.nzeros == 0 )
         strcat( line, " none" );
      else
         for ( i = 0; i < intHeader.nzeros; ++i ) {
            sprintf( zr, "%.7g", realHeader.zeros[2*i  ] );
            sprintf( zi, "%.7g", realHeader.zeros[2*i+1] );
            sprintf( z, " (%.14s,%.14s)", zr, zi );
            l = strlen( z );
            if ( n + l > 80 ) {
               puts( line );
               strcpy( line, "      " );
               n = strlen( line );
            }
            strcat( line, z );
         }
      puts( line );

   }

   return EXIT_SUCCESS;

}
