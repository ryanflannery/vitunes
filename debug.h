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

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

/* log file for debugging */
extern FILE *debug_log;

#ifdef DEBUG

/* debug file logger that goes to the file opened in vitunes.c */
#define DFLOG(format, args...) \
   fprintf(debug_log, "%s.%d (%s): ", __FILE__, __LINE__, __FUNCTION__); \
   fprintf(debug_log, format, ## args); \
   fprintf(debug_log, "\n"); \
   fflush(debug_log);

/* console logger.  goes to stdout.  doesn't work well in curses */
#define DCLOG(format, args...) \
   printf("%d: ", __LINE__); \
   printf(format, ## args); \
   printf("\n"); \
   fflush(stdout);


#else

#define DFLOG(format, args...) 0;
#define DCLOG(format, args...) 0;

#endif


/* for unused arguments */
#if defined(__GNUC__) || defined(__clang__)
#  define UNUSED  __attribute__((__unused__))
#else
#  define UNUSED
#endif

#endif
