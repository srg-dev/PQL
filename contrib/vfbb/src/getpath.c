/******************************************************************************
 *                                                                            *
 * getpath.c - Convert a relative path name to an absolute path name.         *
 *                                                                            *
 * Funtion prototype:                                                         *
 *                                                                            *
 *    void getpath( const char *name, char *path, size_t pathmax );           *
 *                                                                            *
 ******************************************************************************/

/* ANSI-Standard Headers */

#include <stdio.h>		/* FILENAME_MAX, printf() */
#include <stdlib.h>		/* NULL, size_t, free(), malloc() */
#include <string.h>		/* strcat(), strncpy(), strrchr() */

/* POSIX-Standard Headers */

#include <limits.h>		/* PATH_MAX */
#include <unistd.h>		/* _PC_PATH_MAX, getcwd(), pathconf() */

#ifdef PATH_MAX
#if ( PATH_MAX+1 ) > FILENAME_MAX
#define FULLNAME_MAX PATH_MAX+1
#else
#define FULLNAME_MAX FILENAME_MAX
#endif
#endif


/****************************************************************** getpath() */

void getpath( const char *name, char *path, size_t pathmax ) {

#ifdef PATH_MAX
   char wd[FULLNAME_MAX];
#else
   long FULLNAME_MAX;
   char *wd;
#endif
   char *cp;


   if ( ( name == NULL ) || ( path == NULL ) )
      return;

#ifdef _DEBUG
   printf( "getpath(): name=\"%s\"\n", name );
#endif

   if ( name[0] == '/' ) {
      strncpy( path, name, pathmax-1 );
      path[pathmax] = '\0';
      return;
   }

#ifndef PATH_MAX
   FULLNAME_MAX = pathconf( "/", _PC_PATH_MAX ) + 1;
   wd = (char *) malloc( FULLNAME_MAX );
   if ( wd == NULL ) {
      strncpy( path, name, pathmax-1 );
      path[pathmax] = '\0';
      return;
   }
#endif

   if ( getcwd( wd, FULLNAME_MAX ) == NULL ) {
      strncpy( path, name, pathmax-1 );
      path[pathmax] = '\0';
#ifndef PATH_MAX
      free( wd );
#endif
      return;
   }

#ifdef _DEBUG
   printf( "getpath(): wd=\"%s\"\n", wd );
#endif

   while ( name[0] == '.' ) {
      if ( name[1] == '\0' ) {
         ++name;
         break;
      } else if ( name[1] == '/' ) {
         name += 2;
      } else if ( name[1] == '.' ) {
         if ( name[2] == '\0' ) {
            cp = strrchr( wd, '/' );
            if ( cp != NULL )
               *cp = '\0';
            name += 2;
            break;
         } else if ( name[2] == '/' ) {
            cp = strrchr( wd, '/' );
            if ( cp != NULL )
               *cp = '\0';
            name += 3;
         } else
            break;
      } else
         break;
   }

   strncpy( path, wd, pathmax-1 );
   if ( name[0] != '\0' ) {
      strncat( path, "/", pathmax-1 );
      strncat( path, name, pathmax-1 );
      path[pathmax] = '\0';
   }

#ifndef PATH_MAX
   free( wd );
#endif

}
