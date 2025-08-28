/******************************************************************************
 *                                                                            *
 * CLISubs.c - Unix/Win32 command line parser for the dhead program.          *
 *                                                                            *
 * Unix/Win32 command line format:                                            *
 *                                                                            *
 *    $ dhead [ ? | -? | -h | -help ]  or                                     *
 *    $ dhead { -v | -version }        or                                     *
 *    $ dhead DR100-file [ { -o | -output } listing-file[.lis] ]              *
 *                                                                            *
 * Funtion prototypes:                                                        *
 *                                                                            *
 *    int parse_command_line( int argc, char *argv[] );                       *
 *                                                                            *
 * parse_command_line() always returns TRUE.                                  *
 *                                                                            *
 * Note: ANSI C requires argv[0] to be the program name; argv[1] is the first *
 *       command-line argument.  If that is not the  case,  #define FIRST_ARG *
 *       (e.g., in the compiler command) to the correct value.                *
 *                                                                            *
 ******************************************************************************/

/* ANSI-Standard Headers */

#include <stdio.h>		/* FILENAME_MAX */
#include <stdlib.h>		/* NULL */
#include <string.h>		/* strcmp() */

#ifndef FIRST_ARG 		/* Assume ANSI behavior (argv[0] is program)  */
#define FIRST_ARG 1
#endif

#ifndef TRUE
#define TRUE ( 0 == 0 )
#endif
#ifndef FALSE
#define FALSE ( 0 != 0 )
#endif

/* Global Data */

extern int boolCLIHelp;
extern int boolCLIVersion;
extern char *strCLIDR100File;
extern int boolCLIOutput;
extern char *strCLIOutput;


/******************************************************* parse_command_line() */

int parse_command_line( int argc, char *argv[] ) {

   int      intArg;
   int      boolBadUsage;


   boolCLIHelp     = FALSE;
   boolCLIVersion  = FALSE;
   strCLIDR100File = NULL;
   boolCLIOutput   = FALSE;
   strCLIOutput    = NULL;

   if ( argc < FIRST_ARG ) {
      boolCLIHelp = TRUE;
      return TRUE;
   }

   boolBadUsage = FALSE;
   for ( intArg = FIRST_ARG; intArg < argc; ++intArg ) {

      if ( ( strcmp( argv[intArg], "?"     ) == 0 ) ||
           ( strcmp( argv[intArg], "-?"    ) == 0 ) ||
           ( strcmp( argv[intArg], "-h"    ) == 0 ) ||
           ( strcmp( argv[intArg], "-help" ) == 0 ) ) {
         if ( boolCLIHelp ) {
            boolBadUsage = TRUE;
            break;
         }
         boolCLIHelp = TRUE;
         continue;
      }

      if ( ( strcmp( argv[intArg], "-o"      ) == 0 ) ||
           ( strcmp( argv[intArg], "-output" ) == 0 ) ) {
         if ( boolCLIOutput || ( ++intArg >= argc ) ) {
            boolBadUsage = TRUE;
            break;
         }
         boolCLIOutput  = TRUE;
         strCLIOutput = argv[intArg];
         continue;
      }

      if ( ( strcmp( argv[intArg], "-v"       ) == 0 ) ||
           ( strcmp( argv[intArg], "-version" ) == 0 ) ) {
         if ( boolCLIVersion ) {
            boolBadUsage = TRUE;
            break;
         }
         boolCLIVersion = TRUE;
         continue;
      }

      if ( strCLIDR100File != NULL ) {
         boolBadUsage = TRUE;
         break;
      }
      strCLIDR100File = argv[intArg];
      continue;

   }

   if ( !boolBadUsage ) {
      int n = 0;
      if ( boolCLIHelp )
         ++n;
      if ( boolCLIVersion )
         ++n;
      if ( ( strCLIDR100File != NULL ) || boolCLIOutput )
         ++n;
      boolBadUsage = n != 1;
   }

   if ( !( boolCLIHelp || boolCLIVersion ) && !boolBadUsage )
      boolBadUsage = strCLIDR100File == NULL;

   if ( boolBadUsage )
      boolCLIHelp = TRUE;

   return TRUE;

}
