/******************************************************************************
 *                                                                            *
 * system_type.h - Determines the type of system and defines the following:   *
 *                                                                            *
 * System     SYSTEM_TYPE        SYSTEM_TYPE_NAME          Requires(1)        *
 * ---------- ------------------ ------------------------- ------------------ *
 * MacIntosh  SYSTEM_TYPE_MACOS                            macintosh          *
 *    68K     SYSTEM_TYPE_68KMAC "Apple MacIntosh"                            *
 *    PowerPC SYSTEM_TYPE_PPCMAC "Apple MacIntosh PowerPC" && __powerc        *
 * Mac OS X   SYSTEM_TYPE_MACOSX                           __MACH__ &&        *
 *                                                         __APPLE__          *
 *    PowerPC SYSTEM_TYPE_PPCOSX                                              *
 * OpenVMS    SYSTEM_TYPE_VMS                              __vms              *
 *    Alpha   SYSTEM_TYPE_AXPVMS "Compaq OpenVMS Alpha"                       *
 *    VAX     SYSTEM_TYPE_VAXVMS "Compaq OpenVMS VAX"      && __vax           *
 * Unix       SYSTEM_TYPE_UNIX   "Generic Unix"            unix || __unix ||  *
 *                                                         __unix__           *
 *    POSIX   SYSTEM_TYPE_POSIX  "Generic POSIX"           && _POSIX_SOURCE   *
 *    SunOS   SYSTEM_TYPE_SUNOS  "Sun SunOS/Solaris"       && ( sun || __sun )*
 *    Tru64   SYSTEM_TYPE_TRU64  "Compaq Tru64 Unix"       && __digital__     *
 *    Linux   SYSTEM_TYPE_LINUX  "Linux"                   && __linux__       *
 * Windows    SYSTEM_TYPE_WIN32  "Microsoft Windows Win32" _WIN32 ||          *
 *                                                         ( _MWERKS_ &&      *
 *                                                         __INTEL__ )        *
 *                                                                            *
 * Note (1): Strict  ANSI  conformance  requires  that  all predefined macros *
 *           begin with either a single  or  double  underscore.   Therefore, *
 *           SYSTEM_TYPE cannot be determined when strict ANSI conformance is *
 *           requested on systems whose predefined macros are non-standard.   *
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
 * 20-Feb-2001  L. M. Baker     Original version.                             *
 *  9-Mar-2001  L. M. Baker     Add #ifndef _SYSTEM_TYPE_H guard.             *
 * 28-Sep-2004  L. M. Baker     Add support for Mac OS X/PowerPC.             *
 * 30-Nov-2004  L. M. Baker     Add support for Linux.                        *
 *                                                                            *
 ******************************************************************************/

#ifndef _SYSTEM_TYPE_H
#define _SYSTEM_TYPE_H

#define SYSTEM_TYPE_MACOS    0x0100
#define SYSTEM_TYPE_MACOSX   0x0200
#define SYSTEM_TYPE_UNIX     0x0400
#define SYSTEM_TYPE_VMS      0x0800
#define SYSTEM_TYPE_WIN32    0x1000

#define SYSTEM_TYPE_68KMAC ( SYSTEM_TYPE_MACOS  | 0x01 )
#define SYSTEM_TYPE_PPCMAC ( SYSTEM_TYPE_MACOS  | 0x02 )
#define SYSTEM_TYPE_PPCOSX ( SYSTEM_TYPE_MACOSX | 0x03 )
#define SYSTEM_TYPE_POSIX  ( SYSTEM_TYPE_UNIX   | 0x04 )
#define SYSTEM_TYPE_SUNOS  ( SYSTEM_TYPE_UNIX   | 0x05 )
#define SYSTEM_TYPE_TRU64  ( SYSTEM_TYPE_UNIX   | 0x06 )
#define SYSTEM_TYPE_LINUX  ( SYSTEM_TYPE_UNIX   | 0x07 )
#define SYSTEM_TYPE_VAXVMS ( SYSTEM_TYPE_VMS    | 0x08 )
#define SYSTEM_TYPE_AXPVMS ( SYSTEM_TYPE_VMS    | 0x09 )

/* macintosh is predefined by MetroWerks' CodeWarrior C/C++ and Apple's */
/* MrC/MrCpp                                                            */

#ifdef macintosh

/* __powerc is predefined by MetroWerks' CodeWarrior C/C++ and Apple's */
/* MrC/MrCpp                                                           */

#ifdef __powerc
#define SYSTEM_TYPE SYSTEM_TYPE_PPCMAC
#define SYSTEM_TYPE_NAME "Apple MacIntosh PowerPC"
#else /* #ifdef __powerc */
#define SYSTEM_TYPE SYSTEM_TYPE_68KMAC
#define SYSTEM_TYPE_NAME "Apple MacIntosh"
#endif /* #ifdef __powerc */
#endif /* #ifdef macintosh */

/* __MACH__ and __APPLE__ are predefined by Apple's GNU C/C++ compiler */
#if defined(__MACH__) && defined(__APPLE__)
#define SYSTEM_TYPE SYSTEM_TYPE_PPCOSX
#define SYSTEM_TYPE_NAME "Apple MacIntosh OS X"
#endif /* #if defined(__MACH__) && defined(__APPLE__) */

/* unix is predefined by Sun's SunOS 5 SPARCompiler C */
/* __unix is predefined by Sun's SunPRO C */
/* __unix__ is predefined by Compaq's Tru64 Unix C */
/* __unix__ is predefined by GNU's gcc */

#if defined(unix) || defined(__unix) || defined(__unix__)

/* sun is predefined by Sun's SunOS 5 SPARCompiler C */
/* __sun is predefined by Sun's SunPRO C */

#if defined(sun) || defined(__sun)
#define SYSTEM_TYPE SYSTEM_TYPE_SUNOS
#define SYSTEM_TYPE_NAME "Sun SunOS/Solaris"
#endif /* #if defined(sun) || defined(__sun) */

/* __digital__ is predefined by Compaq's Tru64 Unix C */

#ifdef __digital__
#define SYSTEM_TYPE SYSTEM_TYPE_TRU64
#define SYSTEM_TYPE_NAME "Compaq Tru64 Unix"
#endif /* #ifdef __digital__ */

/* __linux__ is predefined by GNU's gcc */

#ifdef __linux__
#define SYSTEM_TYPE SYSTEM_TYPE_LINUX
#define SYSTEM_TYPE_NAME "Linux"
#endif /* #ifdef __linux__ */

#ifndef SYSTEM_TYPE

/* _POSIX_SOURCE is supposed to be predefined by a POSIX application */

#ifdef _POSIX_SOURCE
#define SYSTEM_TYPE SYSTEM_TYPE_POSIX
#define SYSTEM_TYPE_NAME "Generic POSIX"
#else
#define SYSTEM_TYPE SYSTEM_TYPE_UNIX
#define SYSTEM_TYPE_NAME "Generic Unix"
#endif /* #ifdef _POSIX_SOURCE */

#endif /* #ifndef SYSTEM_TYPE */

#endif /* #if defined(unix) || defined(__unix) || defined(__unix__) */

/* __vms is predefined by Compaq C (DEC C) for OpenVMS */

#ifdef __vms

/* __vax is predefined by Compaq C (DEC C) for OpenVMS/VAX (VAX/VMS) */

#ifdef __vax
#define SYSTEM_TYPE SYSTEM_TYPE_VAXVMS
#define SYSTEM_TYPE_NAME "Compaq OpenVMS VAX"
#else
#define SYSTEM_TYPE SYSTEM_TYPE_AXPVMS
#define SYSTEM_TYPE_NAME "Compaq OpenVMS Alpha"
#endif /* #ifdef __vax */

#endif /* #ifdef __vms */

/* _WIN32 is predefined by Microsoft's Visual C++ */
/* _MWERKS_ and __INTEL__ are predefined by MetroWerks' CodeWarrior C/C++ */

#if defined(_WIN32) || ( defined(_MWERKS_) && defined(__INTEL__) )
#define SYSTEM_TYPE SYSTEM_TYPE_WIN32
#define SYSTEM_TYPE_NAME "Microsoft Windows Win32"
#endif /* #if defined(_WIN32) || ( defined(_MWERKS_) && defined(__INTEL__) ) */

#ifndef SYSTEM_TYPE
#error System type cannot be determined
#endif

#endif /* #ifndef _SYSTEM_TYPE_H */
