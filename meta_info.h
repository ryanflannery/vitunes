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

#ifndef META_INFO_H
#define META_INFO_H

#include <ctype.h>
#include <limits.h>
#include <err.h>
#include <errno.h>
#include <stdbool.h> 
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

/* non-baes includes (just TagLib) */
#include <tag_c.h>

#include "debug.h"
#include "enums.h"

/* the character-info fields.  used for all meta-info that's shown */
#define MI_NUM_CINFO     8
#define MI_CINFO_ARTIST  0
#define MI_CINFO_ALBUM   1
#define MI_CINFO_TITLE   2
#define MI_CINFO_TRACK   3
#define MI_CINFO_YEAR    4
#define MI_CINFO_GENRE   5
#define MI_CINFO_LENGTH  6
#define MI_CINFO_COMMENT 7

/* struct used to represent all meta information from a given file */
typedef struct {
   char       *filename;               /* filename of file itself */
   char       *cinfo[MI_NUM_CINFO];    /* character meta info array */
   int         length;                 /* play length in seconds */
   time_t      last_updated;           /* last time info was extracted */
   bool        is_url;                 /* if this is a url */
} meta_info;

/*
 * XXX Note in the above that the playlength is stored both numerically
 * in the member 'length' and as a character string in the cinfo array
 * in the form "hh:mm:ss"
 */

/* array of human-readable names of each CINFO member */
extern const char *MI_CINFO_NAMES[MI_NUM_CINFO];

/* create/destroy meta_info structs */
meta_info *mi_new(void);
void mi_free(meta_info *info);

/* read/write meta_info structs to a file */
void mi_fwrite(meta_info *mi, FILE *fout);
void mi_fread(meta_info *mi,  FILE *fin);

/* used to extract meta info from a media file */
meta_info* mi_extract(const char *filename);


/*****************************************************************************
 * XXX Important Note XXX These functions are used to replace any
 * non-printable characters in a string -or- any of the cinfo fields of a
 * meta-info struct.  This must be done as some files (sadly, some Aphex Twin)
 * contain such characters in their meta info, and these characters correspond
 * to ncurses control sequences, thus mucking-up the display when painted to
 * the screen.  See "vitunes -e help check" for details.
 * This is NOT called explicitly during mi_extract, and MUST be done
 * eleswhere (medialib_db_update, medialib_db_scan_dirs, ecmd_addurl, etc.)
 * This is so that one may use the "check" e-command to see which files have
 * such characters, and then tag them correctly.
 ****************************************************************************/

void str_sanitize(char *s);
void mi_sanitize(meta_info *mi);


/*****************************************************************************
 * Functions used to query meta_info's (i.e. to match them against a given
 * search/filter).  It works by setting-up a global query description which
 * contains a list of tokens in the search and whether or not each token
 * should be matched positively/negatively.
 *
 * After the global query has been set, mi_match() can be used to compare a
 * given meta_info against this global query description.
 ****************************************************************************/

/* structure used to describe what to match meta_info's against */
#define MI_MAX_QUERY_TOKENS   255
typedef struct {
   char *tokens[MI_MAX_QUERY_TOKENS];
   char  match[MI_MAX_QUERY_TOKENS];
   int   ntokens;
   char *raw;  /* a copy of the original, un-tokenized query */
} mi_query_description;

/* flag to indicate if we should include filename when matching */
extern bool mi_query_match_filename;

/* initialize, set, and clear global query description */
void mi_query_init();
bool mi_query_isset();
void mi_query_clear();
void mi_query_add_token(const char *token);

void mi_query_setraw(const char *query);
const char *mi_query_getraw();

/* match a given meta_info/string against the global query description */
bool mi_match(const meta_info *mi);
bool str_match_query(const char *s);


/*****************************************************************************
 * Functions used to sort meta_info's.  These work by setting-up a global
 * sort description that includes the ordering of the CINFO items to sort and
 * which, if any, to sort descending.
 *
 * Once the global sort description has been setup, mi_compare() can be used
 * to compare two meta_info's in a way that works with qsort(3), heapsort(3),
 * or mergesort(3).
 ****************************************************************************/

/* structure used to describe how to sort meta_info structs */
typedef struct {
   int   order[MI_NUM_CINFO];
   bool  descending[MI_NUM_CINFO];
   int   nfields;
} mi_sort_description;

/* initialize, set, and clear global sort description */
void mi_sort_init();
void mi_sort_clear();
int  mi_sort_set(const char *str, const char **errmsg);

/* compare two meta_info's using the global sort description */
int  mi_compare(const void *a, const void *b);


/*****************************************************************************
 * Functions to control how to display meta_info's to the screen.  These
 * setup a global description of the display format, which includes an ordered
 * list of the fields and the width of each field.
 ****************************************************************************/

/* structure used to describe how to display meta_info structs */
typedef struct {
   int       nfields;
   int       order[MI_NUM_CINFO];
   int       widths[MI_NUM_CINFO];
   Direction align[MI_NUM_CINFO];
} mi_display_description;
extern mi_display_description mi_display;

/* initialize and set the global display description */
void  mi_display_init();
int   mi_display_set(const char *str, const char **errmsg);
void  mi_display_reset();

/* convert current display to a string */
char *mi_display_tostr();

/* get total width of display */
int   mi_display_getwidth();

#endif
