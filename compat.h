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
 *
 * Features that are needed on other OS's:
 *
 *    1. strtonum(3)       This is an easy add.
 *                         Required by: Mac OS X, Linux.
 * 
 *    2. fparseln(3)       This is a not-so-easy add.  Damn you, fgetln(3)!
 *                         Required by: Linux.
 */

#ifndef COMPAT_H
#define COMPAT_H

/* Is strtonum(3) required? */
#if defined(__linux) || ( defined(__APPLE__) && defined(__MACH__) )
#  define COMPAT_NEED_STRTONUM
#  include <stdlib.h>
   long long strtonum(const char *, long long, long long, const char **);
#endif

#endif
