/*
 * Copyright (c) 2010, 2011, 2012 Ryan Flannery <ryan.flannery@gmail.com>
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
#include <ncurses.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "paint.h"
#include "uinterface.h"
#include "vitunes.h"

/* error type which depends on the context */
static int error_type;

/*
 * Starts by fetching the needed attributes that depend on the type of the
 * message. It, then, paints it to the command/status window and, if wanted
 * appends the errno message string. A beep is also issued if it's an error
 * message.
 */
static void
error_paint(bool errnoflag, bool iserr, const char *fmt, va_list ap)
{
   int attrs, which;

   which = iserr ? colors.errors : colors.messages;
   attrs = COLOR_PAIR(which);

   werase(ui.command);
   wmove(ui.command, 0, 0);
   wattron(ui.command, attrs);
   vwprintw(ui.command, fmt, ap);

   if (errnoflag)
      wprintw(ui.command, ": %s", strerror(errno));
   if (iserr)
      beep();

   wattroff(ui.command, attrs);
   wrefresh(ui.command);
}

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
 * Prints a fatal message (informational messages are pointless in this context)
 * and terminates the process.
 */
static void
error_cfg(bool errnoflag, bool iserr, const char *fmt, va_list ap)
{
   if (!iserr)
      return;

   endwin();
   error_print(errnoflag, fmt, ap);
   exit(1);
}

/*
 * Prints a message to standard error and terminates the process if it's an
 * error.
 */
static void
error_stderr(bool errnoflag, bool iserr, const char *fmt, va_list ap)
{
   error_print(errnoflag, fmt, ap);
   if (iserr)
      exit(1);
}

/*
 * Check which context we are in and call the function responsible to output the
 * error message.
 */
static void
error_doit(bool errnoflag, bool iserr, const char *fmt, va_list ap)
{
   switch (error_type) {
   case ERROR_CFG:
      error_cfg(errnoflag, iserr, fmt, ap);
      break;
   case ERROR_PAINT:
      error_paint(errnoflag, iserr, fmt, ap);
      break;
   default:
      error_stderr(errnoflag, iserr, fmt, ap);
      break;
   }
}

void
error_init(int type)
{
   error_type = type;
}

/* Outputs a fatal message with the errno message string appended. */
void
fatal(const char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   error_doit(true, true, fmt, ap);
   va_end(ap);
}

/* Outputs a fatal message. */
void
fatalx(const char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   error_doit(false, true, fmt, ap);
   va_end(ap);
}

/* Outputs an informational message with the errno message string appended. */
void
info(const char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   error_doit(true, false, fmt, ap);
   va_end(ap);
}

/* Outputs an informational message. */
void
infox(const char *fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   error_doit(false, false, fmt, ap);
   va_end(ap);
}
