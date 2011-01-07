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

#include "compat.h"

#ifdef COMPAT_NEED_STRTONUM
#  include "compat/strtonum.c"
#endif


#ifdef COMPAT_NEED_FPARSELN
#  define FLOCKFILE(fp)   do { if (__isthreaded) flockfile(fp); } while (0)
#  define FUNLOCKFILE(fp) do { if (__isthreaded) funlockfile(fp); } while (0)
#  include "compat/fgetln.c"
#  define FPARSELN_UNESCESC  0x01
#  define FPARSELN_UNESCCONT 0x02
#  define FPARSELN_UNESCCOMM 0x04
#  define FPARSELN_UNESCREST 0x08
#  define FPARSELN_UNESCALL  0x0f
#  include "compat/fparseln.c"
#endif


#if defined(__linux)
int optreset;
#endif
