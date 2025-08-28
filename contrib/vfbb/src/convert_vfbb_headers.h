/******************************************************************************
 *                                                                            *
 * convert_vfbb_headers.h - Convert the VAX-format integer and real headers   *
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
 *  8-Mar-2001  L. M. Baker     Original version.                             *
 *                              Define upcased and underscore-appended        *
 *                                 variants.                                  *
 *  9-Mar-2001  L. M. Baker     Add #ifndef _CONVERT_VFBB_HEADERS_H guard.    *
 *                                                                            *
 ******************************************************************************/

#ifndef _CONVERT_VFBB_HEADERS_H
#define _CONVERT_VFBB_HEADERS_H

#ifndef FORTRAN_LINKAGE
#define FORTRAN_LINKAGE
#endif

#ifdef UPCASE
#define from_vax_ihdr FROM_VAX_IHDR
#define from_vax_rhdr FROM_VAX_RHDR
#define from_pc_rhdr  FROM_PC_RHDR
#define to_vax_ihdr   TO_VAX_IHDR
#define to_vax_rhdr   TO_VAX_RHDR
#endif

#ifdef APPEND_UNDERSCORE
#define from_vax_ihdr from_vax_ihdr##_
#define from_vax_rhdr from_vax_rhdr##_
#define from_pc_rhdr  from_pc_rhdr##_
#define to_vax_ihdr   to_vax_ihdr##_
#define to_vax_rhdr   to_vax_rhdr##_
#endif

void FORTRAN_LINKAGE from_vax_ihdr( const void *inbuf, void *outbuf );
void FORTRAN_LINKAGE from_vax_rhdr( const void *inbuf, void *outbuf );
void FORTRAN_LINKAGE from_pc_rhdr(  const void *inbuf, void *outbuf );
void FORTRAN_LINKAGE to_vax_ihdr(   const void *inbuf, void *outbuf );
void FORTRAN_LINKAGE to_vax_rhdr(   const void *inbuf, void *outbuf );

#endif /* #ifndef _CONVERT_VFBB_HEADERS_H */
