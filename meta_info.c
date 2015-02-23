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

#include "meta_info.h"

/* human-readable names of all of the string-type meta information values */
const char *MI_CINFO_NAMES[] = {
   "Artist",
   "Album",
   "Title",
   "Track",
   "Year",
   "Genre",
   "Length",
   "Comment"
};

/*
 * Create and return a new meta_info struct.  All memory is allocated, and
 * the resulting pointer should be free(3)'d using mi_free().
 */
meta_info *
mi_new(void)
{
   meta_info *mi;
   int i;

   if ((mi = malloc(sizeof(meta_info))) == NULL)
      err(1, "mi_new: meta_info malloc failed");

   mi->filename = NULL;
   mi->length = 0;
   mi->last_updated = 0;
   mi->is_url = false;

   for (i = 0; i < MI_NUM_CINFO; i++)
      mi->cinfo[i] = NULL;

   return mi;
}


/* Function to free() all memory allocated by a given meta_info struct.  */
void
mi_free(meta_info *mi)
{
   int i;

   if (mi->filename != NULL)
      free(mi->filename);

   for (i = 0; i < MI_NUM_CINFO; i++) {
      if (mi->cinfo[i] != NULL)
         free(mi->cinfo[i]);
   }

   free(mi);
}

/* Function to write a meta_info struct to a file stream.  */
void
mi_fwrite(meta_info *mi, FILE *fout)
{
   static uint16_t lengths[MI_NUM_CINFO + 1];   /* +1 for filename */
   int i;

   /* store all necessary numeric values in the above array */
   lengths[0] = strlen(mi->filename);
   for (i = 0; i < MI_NUM_CINFO; i++) {
      if (mi->cinfo[i] == NULL)
         lengths[i+1] = 0;
      else
         lengths[i+1] = strlen(mi->cinfo[i]);
   }

   /* write */
   fwrite(lengths, sizeof(lengths), 1, fout);
   fwrite(mi->filename, sizeof(char), lengths[0], fout);
   for (i = 0; i < MI_NUM_CINFO; i++) {
      if (mi->cinfo[i] != NULL)
         fwrite(mi->cinfo[i], sizeof(char), lengths[i+1], fout);
   }
   fwrite(&(mi->length),   sizeof(int), 1, fout);
   fwrite(&(mi->last_updated), sizeof(time_t),   1, fout);
   fwrite(&(mi->is_url),       sizeof(bool),     1, fout);

   fflush(fout);
}

/* Function to read a meta_info struct from a file stream */
void
mi_fread(meta_info *mi, FILE *fin)
{
   static uint16_t lengths[MI_NUM_CINFO + 1];   /* +1 for filename */
   int i;

   /* first read all necessary numeric values */
   fread(lengths, sizeof(lengths), 1, fin);

   /* allocate all needed space in the meta_info struct, and zero */
   if ((mi->filename = calloc(lengths[0] + 1, sizeof(char))) == NULL)
      err(1, "mi_fread: calloc filename failed");

   bzero(mi->filename, sizeof(char) * (lengths[0] + 1));

   for (i = 0; i < MI_NUM_CINFO; i++) {
      if (lengths[i+1] > 0) {
         if ((mi->cinfo[i] = calloc(lengths[i+1] + 1, sizeof(char))) == NULL)
            err(1, "mi_fread: failed to calloc cinfo");

         bzero(mi->cinfo[i], sizeof(char) * (lengths[i+1] + 1));
      }
   }

   /* read */
   fread(mi->filename, sizeof(char), lengths[0], fin);
   for (i = 0; i < MI_NUM_CINFO; i++) {
      if (lengths[i+1] > 0)
         fread(mi->cinfo[i], sizeof(char), lengths[i+1], fin);
   }
   fread(&(mi->length),       sizeof(int),      1, fin);
   fread(&(mi->last_updated), sizeof(time_t),   1, fin);
   fread(&(mi->is_url),       sizeof(bool),     1, fin);
}

/* given a number of seconds s, format a "hh:mm::ss" string */
char *
time2str(int s)
{
   static char str[255];
   int hours, minutes, seconds;

   if (s <= 0)
      return "?";

   hours = s / 3600;
   minutes = s % 3600 / 60;
   seconds = s % 60;

   if (hours > 0)
      snprintf(str, sizeof(str), "%i:%02i:%02i", hours, minutes, seconds);
   else if (minutes > 0)
      snprintf(str, sizeof(str), "%i:%02i", minutes, seconds);
   else
      snprintf(str, sizeof(str), "%is", seconds);

   return str;
}


/*
 * Extract meta-information from the provided file, returning a new
 * meta_info* struct if any information was found, NULL if no information
 * was available.
 */
meta_info *
mi_extract(const char *filename)
{
   static char fullname[PATH_MAX];
   const TagLib_AudioProperties *properties;
   TagLib_File *file;
   TagLib_Tag  *tag;
   char *str;

   /* create new, empty meta info struct */
   meta_info *mi = mi_new();

   /* store full filename in meta_info struct */
   bzero(fullname, sizeof(fullname));
   if (realpath(filename, fullname) == NULL)
      err(1, "mi_extract: realpath failed to resolve '%s'", filename);

   if ((mi->filename = strdup(fullname)) == NULL)
      errx(1, "mi_extract: strdup failed for '%s'", fullname);

   /* start extracting fields using TagLib... */

   taglib_set_strings_unicode(false);

   if ((file = taglib_file_new(mi->filename)) == NULL
    || !taglib_file_is_valid(file)) {
      mi_free(mi);
      return NULL;
   }

   /* extract tag-info + audio properties (length) */
   tag = taglib_file_tag(file);
   properties = taglib_file_audioproperties(file);

   /* artist/album/title/genre */
   if ((str = taglib_tag_artist(tag)) != NULL)
      mi->cinfo[MI_CINFO_ARTIST] = strdup(str);

   if ((str = taglib_tag_album(tag)) != NULL)
      mi->cinfo[MI_CINFO_ALBUM] = strdup(str);

   if ((str = taglib_tag_title(tag)) != NULL)
      mi->cinfo[MI_CINFO_TITLE] = strdup(str);

   if ((str = taglib_tag_genre(tag)) != NULL)
      mi->cinfo[MI_CINFO_GENRE] = strdup(str);

   if ((str = taglib_tag_comment(tag)) != NULL)
      mi->cinfo[MI_CINFO_COMMENT] = strdup(str);

   if (mi->cinfo[MI_CINFO_ARTIST] == NULL
   ||  mi->cinfo[MI_CINFO_ALBUM] == NULL
   ||  mi->cinfo[MI_CINFO_TITLE] == NULL
   ||  mi->cinfo[MI_CINFO_GENRE] == NULL
   ||  mi->cinfo[MI_CINFO_COMMENT] == NULL)
      err(1, "mi_extract: strdup for CINFO failed");

   /* track number */
   if (taglib_tag_track(tag) > 0) {
      if (asprintf(&(mi->cinfo[MI_CINFO_TRACK]), "%3i", taglib_tag_track(tag)) == -1)
         err(1, "mi_extract: asprintf failed for CINFO_TRACK");
   }

   /* year */
   if (taglib_tag_year(tag) > 0) {
      if (asprintf(&(mi->cinfo[MI_CINFO_YEAR]), "%i", taglib_tag_year(tag)) == -1)
         err(1, "mi_extract: asprintf failed for CINFO_YEAR");
   }

   /* playlength in seconds (will be 0 if unavailable) */
   mi->length = taglib_audioproperties_length(properties);
   if (mi->length > 0) {
      if ((mi->cinfo[MI_CINFO_LENGTH] = strdup(time2str(mi->length))) == NULL)
         err(1, "mi_extract: strdup failed for CINO_LENGTH");
   }

   /* record the time we extracted this info */
   time(&mi->last_updated);

   /* cleanup */
   taglib_tag_free_strings();
   taglib_file_free(file);

   return mi;
}

/*****************************************************************************
 * The sanitation routines
 ****************************************************************************/
void
str_sanitize(char *s)
{
   size_t i;
   char   c;

   for (i = 0; i < strlen(s); i++) {
      c = s[i];
      if (!isdigit(c) && !isalpha(c) && !ispunct(c) && c != ' ')
         s[i] = '?';
   }
}

void
mi_sanitize(meta_info *mi)
{
   int i;
   for (i = 0; i < MI_NUM_CINFO; i++) {
      if (mi->cinfo[i] != NULL)
         str_sanitize(mi->cinfo[i]);
   }
}


/*****************************************************************************
 * The mi_query_* and mi_compare stuff.
 ****************************************************************************/

/* the global query description */
mi_query_description _mi_query;

/* global flag to indicate if we should match against filename in queires */
bool mi_query_match_filename;

/* initialize the query structures */
void
mi_query_init(void)
{
   int i;

   for (i = 0; i < MI_MAX_QUERY_TOKENS; i++)
      _mi_query.tokens[i] = NULL;

   mi_query_match_filename = true;
   _mi_query.raw = NULL;
   _mi_query.ntokens = 0;
}

/* determine if a query has been set */
bool
mi_query_isset(void)
{
   return _mi_query.ntokens != 0;
}

/* free the query structures */
void
mi_query_clear(void)
{
   int i;

   for (i = 0; i < MI_MAX_QUERY_TOKENS; i++) {
      if (_mi_query.tokens[i] != NULL) {
         free(_mi_query.tokens[i]);
         _mi_query.tokens[i] = NULL;
      }
   }

   if (_mi_query.raw != NULL) {
      free(_mi_query.raw);
      _mi_query.raw = NULL;
   }

   _mi_query.ntokens = 0;
}

/* add a token to the current query description */
void
mi_query_add_token(const char *token)
{
   if (_mi_query.ntokens == MI_MAX_QUERY_TOKENS)
      errx(1, "mi_query_add_token: reached shamefull limit");

   /* match or no? */
   if (token[0] == '!') {
      _mi_query.match[_mi_query.ntokens] = false;
      token++;
   } else
      _mi_query.match[_mi_query.ntokens] = true;

   /* copy token */
   if ((_mi_query.tokens[_mi_query.ntokens++] = strdup(token)) == NULL)
      err(1, "mi_query_add_token: strdup failed");
}

void
mi_query_setraw(const char *query)
{
   if (_mi_query.raw != NULL)
      free(_mi_query.raw);

   if ((_mi_query.raw = strdup(query)) == NULL)
      err(1, "mi_query_setraw: query strdup failed");
}

const char *
mi_query_getraw(void)
{
   return _mi_query.raw;
}

/* match a given meta_info struct against the global query */
bool
mi_match(const meta_info *mi)
{
   bool  matches;
   int   i, j;

   for (i = 0; i < _mi_query.ntokens; i++) {

      matches = false;

      /* does the filename match? */
      if (mi_query_match_filename) {
         if ((strcasestr(mi->filename, _mi_query.tokens[i])) != NULL)
            matches = true;
      }

      /* do any of the CINFO fields match? */
      for (j = 0; !matches && j < MI_NUM_CINFO; j++) {
         if (mi->cinfo[j] == NULL)
            continue;

         if ((strcasestr(mi->cinfo[j], _mi_query.tokens[i])) != NULL)
            matches = true;
      }

      if (!matches && _mi_query.match[i])
         return false;
      if (matches && !_mi_query.match[i])
         return false;
   }

   return true;
}

/*
 * Match any given string against the current query.  Note that this is ONLY
 * used when searching the library window.
 */
bool
str_match_query(const char *s)
{
   bool matches;
   int  i;

   for (i = 0; i < _mi_query.ntokens; i++) {
      matches = false;
      if (strcasestr(s, _mi_query.tokens[i]) != NULL)
         matches = true;
      if (!matches && _mi_query.match[i])
         return false;
      if (matches && !_mi_query.match[i])
         return false;
   }

   return true;
}


/*****************************************************************************
 * mi_sort_* and mi_compare stuff
 ****************************************************************************/

/* global sort description */
mi_sort_description _mi_sort;

/* initialize the sort ordering to what i like */
void
mi_sort_init(void)
{
   _mi_sort.order[0] = MI_CINFO_ARTIST;
   _mi_sort.order[1] = MI_CINFO_ALBUM;
   _mi_sort.order[2] = MI_CINFO_TRACK;
   _mi_sort.order[3] = MI_CINFO_TITLE;

   _mi_sort.descending[0] = false;
   _mi_sort.descending[1] = false;
   _mi_sort.descending[2] = false;
   _mi_sort.descending[3] = false;

   _mi_sort.nfields = 4;
}

/* clear the current sort */
void
mi_sort_clear(void)
{
   _mi_sort.nfields = 0;
}

/* Set the current sort description to what is provided in the given string.
 * The format of the string is a comma-seperated list of field-names, possibly
 * preceeded with a dash (-) to indicate that the field should be sorted
 * descending.
 * Example:
 *       s = "artist,album,track,title"
 * Would set the global sort structure to sort meta_info structs by first
 * artist, then album, then track, then title.
 * Example:
 *       s = "artist,-year"
 * Would set the global sort structure to sort meta_info struct first by
 * artist name, then by year descending (showing the most recent work of
 * a given artist first).
 *
 * RETURNS:
 *    0 if s is parsed without error.  1 otherwise.
 */
int
mi_sort_set(const char *s, const char **errmsg)
{
   mi_sort_description new_sort;
   bool   found;
   char  *token;
   char  *line;
   char  *copy;
   int    idx;
   int    i;

   /* error message to be returned */
   const char *ERRORS[2] = {
      "Too many fields",
      "Unknown field name"
   };
   *errmsg = NULL;

   if ((line = strdup(s)) == NULL)
      err(1, "mi_sort_set: sort strdup failed");

   idx = 0;
   copy = line;

   while ((token = strsep(&line, ",")) != NULL) {

      if (strlen(token) == 0)
         continue;

      if (idx >= MI_NUM_CINFO) {
         *errmsg = ERRORS[0];
         goto err;
      }

      new_sort.descending[idx] = (token[0] == '-');
      if (token[0] == '-')
         token++;

      found = false;
      for (i = 0; i < MI_NUM_CINFO && !found; i++) {
         if (strcasecmp(token, MI_CINFO_NAMES[i]) == 0) {
            new_sort.order[idx] = i;
            found = true;
         }
      }

      if (!found) {
         *errmsg = ERRORS[1];
         goto err;
      }

      idx++;
   }


   /* copy new sort description into global one */
   new_sort.nfields = idx;
   for (idx = 0; idx < new_sort.nfields; idx++) {
      _mi_sort.order[idx]      = new_sort.order[idx];
      _mi_sort.descending[idx] = new_sort.descending[idx];
   }
   _mi_sort.nfields = new_sort.nfields;

   free(copy);
   return 0;

err:
   free(copy);
   return 1;
}

/*
 * Compare two meta_info structs using the global sort description
 * Note that this function is suitable for passing to qsort(3) and the like.
 * TODO investigate way to ignore stuff like a starting "The" or "A" when
 * sorting.  Wait, do I want this?
 */
int
mi_compare(const void *A, const void *B)
{
   int field;
   int ret;
   int i;

   const meta_info **a2 = (const meta_info**) A;
   const meta_info **b2 = (const meta_info**) B;
   const meta_info *a = *a2;
   const meta_info *b = *b2;

   for (i = 0; i < _mi_sort.nfields; i++) {
      field = _mi_sort.order[i];

      if (a->cinfo[field] == NULL && b->cinfo[field] == NULL)
         return 0;
      if (a->cinfo[field] != NULL && b->cinfo[field] == NULL)
         return (_mi_sort.descending[i] ? 1 : -1);
      if (a->cinfo[field] == NULL && b->cinfo[field] != NULL)
         return (_mi_sort.descending[i] ? -1 : 1);

      ret = strcasecmp(a->cinfo[field], b->cinfo[field]);
      if (ret != 0)
         return (_mi_sort.descending[i] ? -1 * ret : ret);
   }

   return 0;
}


/*****************************************************************************
 * mi_display_* stuff
 ****************************************************************************/

/* global display description */
mi_display_description mi_display;

/* initialize global display description to what i like */
void
mi_display_init(void)
{
   mi_display.nfields = 6;

   mi_display.order[0] = MI_CINFO_ARTIST;
   mi_display.order[1] = MI_CINFO_ALBUM;
   mi_display.order[2] = MI_CINFO_TITLE;
   mi_display.order[3] = MI_CINFO_TRACK;
   mi_display.order[4] = MI_CINFO_YEAR;
   mi_display.order[5] = MI_CINFO_LENGTH;

   mi_display.widths[0] = 20;
   mi_display.widths[1] = 20;
   mi_display.widths[2] = 20;
   mi_display.widths[3] = 4;
   mi_display.widths[4] = 4;
   mi_display.widths[5] = 9;

   mi_display.align[0] = LEFT;
   mi_display.align[1] = LEFT;
   mi_display.align[2] = LEFT;
   mi_display.align[3] = RIGHT;
   mi_display.align[4] = RIGHT;
   mi_display.align[5] = RIGHT;
}

/* reset the display to what i like */
void
mi_display_reset(void)
{
   mi_display_init();
}

/* return the total width of the current display description */
int
mi_display_getwidth(void)
{
   int   sum;
   int   i;

   sum = 0;
   for (i = 0; i < mi_display.nfields; i++)
      sum += mi_display.widths[i] + 1;

   return sum;
}

/* convert current display to a string for showing elsewhere */
char *
mi_display_tostr(void)
{
   /*
    * NOTE: for the below "dirty hack" ... it would require some *huge*
    * field-widths to overload this, since there can only be MI_NUM_CINFO
    * fields.  low-priority
    */
   static char s[1000]; /* XXX dirty hack for now */
   char *c;
   int   field, cinfo, size, num;

   size = sizeof(s);

   c = s;
   for (field = 0; field < mi_display.nfields && size > 0; field++) {

      if (mi_display.align[field] == RIGHT) {
         *c = '-';
         c++;
      }

      cinfo = mi_display.order[field];

      num = snprintf(c, size, "%s.%i%s",
         MI_CINFO_NAMES[cinfo],
         mi_display.widths[field],
         (field + 1 == mi_display.nfields ? "" : ","));

      size -= num;
      c += num;
   }

   return s;
}

/*
 * Set the global display description to the format specified in the given
 * string.
 */
int
mi_display_set(const char *display, const char **errmsg)
{
   mi_display_description new_display;
   bool  found;
   char *s, *copy;
   char *token;
   char *num;
   int   idx;
   int   i;

   /* error message to be returned */
   const char *ERRORS[3] = {
      "Too many fields",
      "Missing '.' and size field",
      "Unknown field name"
   };

   *errmsg = NULL;
   new_display.nfields = 0;

   if ((s = strdup(display)) == NULL)
      err(1, "mi_display_set: display strdup failed");

   copy = s;
   idx = 0;

   while ((token = strsep(&s, ",")) != NULL) {

      if (strlen(token) == 0)
         continue;

      /* make sure we don't have too many fields */
      if (idx >= MI_NUM_CINFO) {
         *errmsg = ERRORS[0];
         goto err;
      }

      /* determine alignment (if '-' is present it's right, otherwise left) */
      if (token[0] == '-') {
         new_display.align[idx] = RIGHT;
         token++;
      } else
         new_display.align[idx] = LEFT;

      /* make sure there's a number part to the field */
      if ((num = strstr(token, ".")) == NULL) {
         *errmsg = ERRORS[1];
         goto err;
      }

      *num = '\0';
      num++;

      new_display.widths[idx] = strtonum(num, 1, 1000, errmsg);
      if (*errmsg != NULL)
         goto err;

      /* get the field name */
      found = false;
      for (i = 0; i < MI_NUM_CINFO && !found; i++) {
         if (strcasecmp(token, MI_CINFO_NAMES[i]) == 0) {
            new_display.order[idx] = i;
            found = true;
         }
      }

      if (!found) {
         *errmsg = ERRORS[2];
         goto err;
      }

      idx++;
      new_display.nfields++;
   }

   /* copy results into global display description */
   for (idx = 0; idx < new_display.nfields; idx++) {
      mi_display.order[idx]  = new_display.order[idx];
      mi_display.widths[idx] = new_display.widths[idx];
      mi_display.align[idx] = new_display.align[idx];
   }
   mi_display.nfields = new_display.nfields;

   free(copy);
   return 0;

err:
   free(copy);
   return 1;
}
