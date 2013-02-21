/*
 * Copyright (c) 2013 Tiago Cunha <tcunha@gmx.com>
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

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "vitunes.h"

/*
 * Prints the provided error message to standard error and, if wanted, appends
 * the errno message string.
 */
static void
error_print(bool errnoflag, const char *fmt, va_list ap)
{
   fprintf(stderr, "%s: ", progname);
   vfprintf(stderr, fmt, ap);

   if (errnoflag)
      fprintf(stderr, ": %s", strerror(errno));
   fputs("\n", stderr);
}

/*
 * Outputs a fatal message, appends the errno message string and terminates the
 * process.
 */
void
fatal(const char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   error_print(true, fmt, ap);
   va_end(ap);
   exit(1);
}

/* Outputs a fatal message and terminates the process. */
void
fatalx(const char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   error_print(false, fmt, ap);
   va_end(ap);
   exit(1);
}

/* Outputs an informational message with the errno message string appended. */
void
info(const char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   error_print(true, fmt, ap);
   va_end(ap);
}

/* Outputs an informational message. */
void
infox(const char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   error_print(false, fmt, ap);
   va_end(ap);
}
