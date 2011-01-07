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
 * Compatibility goo for other OS's.  Specifically,
 *
 *    1. Mac OS X
 *          -  Requires strtonum(3) implementation.
 *
 *    2. Linux
 *          -  Requires strtonum(3) implementation.
 *          -  Requires fparseln(3) implementation.
 *
 * So far, all compatibility code is from the OpenBSD projects and each
 * file has its own license.
 *
 * Backed out Linux support on 7 Jan. 2011.
 */

#ifndef COMPAT_H
#define COMPAT_H

/* strtonum(3) implementation */
#if defined(__linux) || ( defined(__APPLE__) && defined(__MACH__) )
#  define COMPAT_NEED_STRTONUM
#  include <stdlib.h>
   long long strtonum(const char *, long long, long long, const char **);
#endif

#endif
