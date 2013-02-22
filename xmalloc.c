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
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "xmalloc.h"

void
xasprintf(char **ret, const char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   if (vasprintf(ret, fmt, ap) == -1 || *ret == NULL)
      diex("vasprintf");
   va_end(ap);
}

void *
xcalloc(size_t nmemb, size_t size)
{
   void *ptr;

   if (nmemb == 0)
      diex("xcalloc: zero objects");
   if (size == 0)
      diex("xcalloc: zero size");
   if (nmemb > SIZE_MAX / size) {
      errno = ENOMEM;
      die("xcalloc: overflow");
   }
   if ((ptr = calloc(nmemb, size)) == NULL)
      die("calloc");

   return (ptr);
}

void *
xmalloc(size_t size)
{
   void *ptr;

   if (size == 0)
      diex("xmalloc: zero size");
   if ((ptr = malloc(size)) == NULL)
      die("malloc");

   return (ptr);
}

void *
xrealloc(void *ptr, size_t nmemb, size_t size)
{
   void *newptr;

   if (nmemb == 0)
      diex("xrealloc: zero objects");
   if (size == 0)
      diex("xrealloc: zero size");
   if (nmemb > SIZE_MAX / size) {
      errno = ENOMEM;
      die("xrealloc: overflow");
   }
   if ((newptr = realloc(ptr, nmemb * size)) == NULL)
      die("realloc");

   return (newptr);
}

char *
xstrdup(const char *s)
{
   char *ptr;

   if (s == NULL)
      diex("xstrdup: null pointer");
   if ((ptr = strdup(s)) == NULL)
      die("strdup");

   return (ptr);
}
