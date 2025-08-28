/******************************************************************************
 *                                                                            *
 * libvfbb.h - VFBB file routines                                             *
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
 *  9-Mar-2001  L. M. Baker     Add #ifndef _LIBVFBB_H guard.                 *
 *                                                                            *
 ******************************************************************************/

#ifndef _LIBVFBB_H
#define _LIBVFBB_H

/* VFBB File Conversion Routines */

#include "bbfile.h"
#include "convert_vfbb_headers.h"

/* VAX Data Conversion Routines */

#include "convert_vax_data.h"

#ifndef FORTRAN_LINKAGE
#define FORTRAN_LINKAGE
#endif

#ifdef UPCASE
#define is_little_endian IS_LITTLE_ENDIAN
#endif

#ifdef APPEND_UNDERSCORE
#define is_little_endian is_little_endian##_
#endif

int FORTRAN_LINKAGE is_little_endian();

#endif /* #ifndef _LIBVFBB_H */
