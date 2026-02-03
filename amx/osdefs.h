/* __MSDOS__    set when compiling for DOS (not Windows)
 * _Windows     set when compiling for any version of Microsoft Windows
 * __WIN32__    set when compiling for Windows95 or WindowsNT (32 bit mode)
 * __32BIT__    set when compiling in 32-bit "flat" mode (DOS or Windows)
 *
 * Copyright 1998-2005, ITB CompuPhase, The Netherlands.
 * info@compuphase.com.
 */

#ifndef _OSDEFS_H
#define _OSDEFS_H

/* Map compiler-specific macros to Borland C++ style macros */
#if defined(__WATCOMC__)
#  if defined(__WINDOWS__) || defined(__NT__)
#    define _Windows    1
#  endif
#  ifdef __386__
#    define __32BIT__   1
#  endif
#  if defined(_Windows) && defined(__32BIT__)
#    define __WIN32__   1
#  endif
#elif defined(_MSC_VER)
#  if defined(_WINDOWS) || defined(_WIN32)
#    define _Windows    1
#  endif
#  ifdef _WIN32
#    define __WIN32__   1
#    define __32BIT__   1
#  endif
#endif

/* Include endian headers if available */
#if defined __FreeBSD__
   #include <sys/endian.h>
#elif defined LINUX
   #include <endian.h>
#endif

/* Linux endian definitions if not present */
#if !defined BIG_ENDIAN
  #define BIG_ENDIAN    4321
#endif
#if !defined LITTLE_ENDIAN
  #define LITTLE_ENDIAN 1234
#endif

/* educated guess, BYTE_ORDER is undefined, i386 is common => little endian */
#if !defined BYTE_ORDER
  #if defined UCLINUX
    #define BYTE_ORDER BIG_ENDIAN
  #else
    #define BYTE_ORDER LITTLE_ENDIAN
  #endif
#endif

/* Directory separator character */
#if defined __MSDOS__ || defined __WIN32__ || defined _Windows
  #define DIRSEP_CHAR '\\'
#elif defined macintosh
  #define DIRSEP_CHAR ':'
#else
  #define DIRSEP_CHAR '/'   /* directory separator character */
#endif

/* -------------------
   MAX_PATH / AMX_MAX_PATH
   ------------------- */

#if !defined(_MAX_PATH)

    /* Windows */
    #if defined(_WIN32) || defined(__WIN32__) || defined(_Windows)
        #include <stdlib.h>
        #ifndef _MAX_PATH
            #define _MAX_PATH 260
        #endif

    /* Linux / Unix */
    #else
        #include <limits.h>
        #if defined(PATH_MAX)
            #define _MAX_PATH PATH_MAX
        #elif defined(_POSIX_PATH_MAX)
            #define _MAX_PATH _POSIX_PATH_MAX
        #else
            #define _MAX_PATH 1024
        #endif
    #endif

#endif

#define AMX_MAX_PATH _MAX_PATH

#endif  /* _OSDEFS_H */
