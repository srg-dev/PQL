/******************************************************************************
 *                                                                            *
 * bbfile.h - Fortran-callable functions to read VFBB files                   *
 *                                                                            *
 *                                                                            *
 * bbdata.c - Fortran-callable function to read a VFBB file's time-series     *
 *                                                                            *
 *    C format (see note for OpenVMS):                                        *
 *                                                                            *
 *       int bbdata( const char *vfbb_file,   VFBB file name                  *
 *                   const int *nfile,        No. of characters in vfbb_file  *
 *                   const int *first_pt,     Index (from 1) of first point   *
 *                   const int *last_pt,      Index of last point (0=all)     *
 *                   double unscaled_data[],  Unscaled time series data       *
 *                   double *scale_factor,    Engineering units multiplier    *
 *                   int *npt );              No. of points returned          *
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
 *       Integer    nfile                     No. of characters in vfbb_file  *
 *       Character  vfbb_file*(nfile)         VFBB file name                  *
 *       Integer    first_pt                  Index (from 1) of first point   *
 *       Integer    last_pt                   Index of last point (0=all)     *
 *       Double Precision unscaled_data(*)    Unscaled time series data       *
 *       Double Precision scale_factor        Engineering units multiplier    *
 *       Integer    npt                       No. of points returned          *
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
 * bbihdr.c - Fortran-callable function to read a VFBB file's integer headers *
 *                                                                            *
 *    C format (see note for OpenVMS):                                        *
 *                                                                            *
 *       int bbihdr( const char *vfbb_file,   VFBB file name                  *
 *                   const int *nfile,        No. of characters in vfbb_file  *
 *                   const int *ihdr_no,      Integer header no. (from 1)     *
 *                   int ihdr[256] );         Integer header                  *
 *                                                                            *
 *       Returns 0 if no errors.  Otherwise, -1: bad calling argument, -2:    *
 *       memory allocation error, -3: ihdr_no does not exist, or the value of *
 *       the C Run-Time Library variable, errno.                              *
 *                                                                            *
 *       Note: On OpenVMS, the first argument is a pointer to a string        *
 *             descriptor, const struct dsc$descriptor_s *vfbb_file.          *
 *                                                                            *
 *    Fortran format:                                                         *
 *                                                                            *
 *       Integer Function BBIHDR( vfbb_file, nfile, ihdr_no, ihdr )           *
 *       Integer    nfile                     No. of characters in vfbb_file  *
 *       Character  vfbb_file*(nfile)         VFBB file name                  *
 *       Integer    ihdr_no                   Integer header no. (from 1)     *
 *       Integer    ihdr(256)                 Integer header                  *
 *                                                                            *
 *       Returns 0 if no errors.  Otherwise, -1: bad calling argument, -2:    *
 *       memory allocation error, -3: ihdr_no does not exist, or the value of *
 *       the C Run-Time Library variable, errno.                              *
 *                                                                            *
 *    MATLAB format:                                                          *
 *                                                                            *
 *       ihdr = bbihdr( 'vfbb_file' [ , ihdr_no ] );                          *
 *                                                                            *
 *                                                                            *
 * bbrhdr.c - Fortran-callable function to read a VFBB file's real headers    *
 *                                                                            *
 *    C format (see note for OpenVMS):                                        *
 *                                                                            *
 *       int bbrhdr( const char *vfbb_file,   VFBB file name                  *
 *                   const int *nfile,        No. of characters in vfbb_file  *
 *                   const int *rhdr_no,      Real header no. (from 1)        *
 *                   float rhdr[128] );       Real header                     *
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
 *       Integer    nfile                     No. of characters in vfbb_file  *
 *       Character  vfbb_file*(nfile)         VFBB file name                  *
 *       Integer    rhdr_no                   Real header no. (from 1)        *
 *       Real       rhdr(128)                 Real header                     *
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
 * bbthdr.c - Fortran-callable function to read a VFBB file's text headers    *
 *                                                                            *
 *    C format (see note for OpenVMS):                                        *
 *                                                                            *
 *       int bbthdr( const char *vfbb_file,   VFBB file name                  *
 *                   const int *nfile,        No. of characters in vfbb_file  *
 *                   const int *thdr_no,      Text header no. (from 1)        *
 *                   char thdr[512] );        Text header                     *
 *                                                                            *
 *       Returns 0 if no errors.  Otherwise, -1: bad calling argument, -2:    *
 *       memory allocation error, -3: thdr_no does not exist, or the value of *
 *       the C Run-Time Library variable, errno.                              *
 *                                                                            *
 *       Note: On OpenVMS, the first and last arguments are pointers to       *
 *             string descriptors, const struct dsc$descriptor_s *vfbb_file   *
 *             and struct dsc$descriptor_s *thdr, respectively.               *
 *                                                                            *
 *    Fortran format:                                                         *
 *                                                                            *
 *       Integer Function BBTHDR( vfbb_file, nfile, thdr_no, thdr )           *
 *       Integer    nfile                     No. of characters in vfbb_file  *
 *       Character  vfbb_file*(nfile)         VFBB file name                  *
 *       Integer    thdr_no                   Text header no. (from 1)        *
 *       Character  thdr*512                  Text header                     *
 *                                                                            *
 *       Returns 0 if no errors.  Otherwise, -1: bad calling argument, -2:    *
 *       memory allocation error, -3: thdr_no does not exist, or the value of *
 *       the C Run-Time Library variable, errno.                              *
 *                                                                            *
 *    MATLAB format:                                                          *
 *                                                                            *
 *       thdr = bbthdr( 'vfbb_file' [ , thdr_no ] );                          *
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
 *  7-Mar-2001  L. M. Baker     Original version.                             *
 *  8-Mar-2001  L. M. Baker     Define upcased and underscore-appended        *
 *                                 variants.                                  *
 *  9-Mar-2001  L. M. Baker     Add #ifndef _BBFILE_H guard.                  *
 *                                                                            *
 ******************************************************************************/

#ifndef _BBFILE_H
#define _BBFILE_H

#include "system_type.h"	/* System Type (MAC, TRU64, UNIX, VMS, WIN32) */

#ifndef FORTRAN_LINKAGE
#define FORTRAN_LINKAGE
#endif

#ifdef UPCASE
#define bbdata BBDATA
#define bbihdr BBIHDR
#define bbrhdr BBRHDR
#define bbthdr BBTHDR
#endif

#ifdef APPEND_UNDERSCORE
#define bbdata bbdata##_
#define bbihdr bbihdr##_
#define bbrhdr bbrhdr##_
#define bbthdr bbthdr##_
#endif

#if ( SYSTEM_TYPE & SYSTEM_TYPE_VMS )

#include <descrip.h>                                    /* dsc$descriptor_s{} */

int bbdata( const struct dsc$descriptor_s *vfbb_file, const int *nfile,
            const int *first_pt, const int *last_pt, double unscaled_data[],
            double *scale_factor, int *npt );
int bbihdr( const struct dsc$descriptor_s *vfbb_file, const int *nfile,
            const int *ihdr_no, int ihdr[256] );
int bbrhdr( const struct dsc$descriptor_s *vfbb_file, const int *nfile,
            const int *rhdr_no, float rhdr[128] );
int bbthdr( const struct dsc$descriptor_s *vfbb_file, const int *nfile,
            const int *thdr_no, struct dsc$descriptor_s *thdr );

#else

int FORTRAN_LINKAGE bbdata( const char *vfbb_file, const int *nfile,
                            const int *first_pt, const int *last_pt,
                            double unscaled_data[], double *scale_factor,
                            int *npt );
int FORTRAN_LINKAGE bbihdr( const char *vfbb_file, const int *nfile,
                            const int *ihdr_no, int ihdr[256] );
int FORTRAN_LINKAGE bbrhdr( const char *vfbb_file, const int *nfile,
                            const int *rhdr_no, float rhdr[128] );
int FORTRAN_LINKAGE bbthdr( const char *vfbb_file, const int *nfile,
                            const int *thdr_no, char thdr[512] );

#endif

#endif /* #ifndef _BBFILE_H */
