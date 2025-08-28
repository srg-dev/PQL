/******************************************************************************
 *                                                                            *
 * is_little_endian.c - Predicate  for determining if machine architecture is *
 *                      "little-endian", i.e., integers are stored  low-order *
 *                      to  high-order in ascending memory addresses (ala the *
 *                      DEC PDP-11 and VAX family and the Intel x86 family).  *
 *                                                                            *
 *                                                                            *
 * Author:  Lawrence M. Baker                                                 *
 *          U.S. Geological Survey                                            *
 *          345 Middlefield Road  MS977                                       *
 *          Menlo Park, CA  94025                                             *
 *          baker@usgs.gov                                                    *
 *                                                                            *
 * Reference: Danny Kalev, 1999, The ANSI/ISO C++ Professional Programmer's   *
 *            Handbook, Que Corporation, "Detecting a Machine's Endian-ness"  *
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
 * 22-Feb-2001  L. M. Baker     Original version.                             *
 *                                                                            *
 ******************************************************************************/

#if defined( __VMS ) && defined( __DECC )
#pragma module is_little_endian
#endif

#include "libvfbb.h"

int FORTRAN_LINKAGE is_little_endian() {

   static const union probe {
      unsigned int num;
      unsigned char bytes[sizeof( unsigned int)];
   } p = { 1U };              /* Initialize first member of p with unsigned 1 */

   return ( p.bytes[0] == 1U );    /* p.bytes[0] is 0 on a big endian machine */

}
