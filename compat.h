/*
 * Copyright (c) 2010, 2011 Ryan Flannery <ryan.flannery@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Compatibility goo for other OS's.  These files are intended to add
 * support for various FEATURES that other OS's do not support.  Kinda'
 * like what autoconf is supposed to do, but doesn't.
 *
 * These files consist of the following:
 *
 *    compat.h       This file, which contains the main feature-goo, along
 *                   with any OS specific goo.  See below for details.
 *
 *    compat.c       The counterpart to this file, which includes all the
 *                   necessary #includes of compat/ stuff to add various
 *                   features.
 *
 *    compat/        The directory containing all OpenBSD-specific
 *                   features of vitunes that may be needed on other OS's.
 *
 */

#ifndef COMPAT_H
#define COMPAT_H

/* OpenBSD has fparseln(3), but it must be included thusly */
#if defined(__OpenBSD__)
#  include <stdio.h>
#  include <util.h>
#endif

/* FreeBSD has fparseln(3), but it must be included thusly */
#if defined(__FreeBSD__)
#  include <stdio.h>
#  include <sys/types.h>
#  include <libutil.h>
#endif

/* Mac OS X needs strtonum(3) */
#if defined(__APPLE__) && defined(__MACH__)
#  define COMPAT_NEED_STRTONUM
#endif

/* Linux needs the following.. */
#if defined(__linux)
#  define COMPAT_NEED_FPARSELN
#  define COMPAT_NEED_OPTRESET
#  define COMPAT_NEED_STRLCAT
#  define COMPAT_NEED_STRTONUM

/* still unsure/curious about these last two */
#  ifndef u_short
   typedef unsigned short u_short;
#  endif
#  ifndef PATH_MAX
#     include <linux/limits.h>
#  else
#     define PATH_MAX 4096
#  endif
#endif


/* Now add necessary prototypes... */

#ifdef COMPAT_NEED_FPARSELN
#  include <stdio.h>
   char *fparseln(FILE *fp, size_t *size, size_t *lineno,
         const char str[3], int flags);
#endif

#ifdef COMPAT_NEED_OPTRESET
   extern int optreset;
#endif

#ifdef COMPAT_NEED_STRLCAT
   size_t strlcat(char *dst, const char *src, size_t siz);
   size_t strlcpy(char *dst, const char *src, size_t siz);
#endif

#ifdef COMPAT_NEED_STRTONUM
   long long strtonum(const char *, long long, long long, const char **);
#endif

#endif
