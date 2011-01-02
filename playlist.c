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

#include "playlist.h"

int history_size = DEFAULT_HISTORY_SIZE;

void
playlist_increase_capacity(playlist *p)
{
   meta_info **new_files;
   size_t      nbytes;

   p->capacity += PLAYLIST_CHUNK_SIZE;
   nbytes = p->capacity * sizeof(meta_info*);
   if ((new_files = realloc(p->files, nbytes)) == NULL)
      err(1, "%s: failed to realloc(3) files", __FUNCTION__);

   p->files = new_files;
}

/*
 * Allocate a new playlist and return a pointer to it.  The resulting
 * structure must be free(2)'d using playlist_free().
 */
playlist *
playlist_new(void)
{
   playlist *p;
   
   if ((p = malloc(sizeof(playlist))) == NULL)
      err(1, "playlist_new: failed to allocate playlist");

   if ((p->files = calloc(PLAYLIST_CHUNK_SIZE, sizeof(meta_info*))) == NULL)
      err(1, "playlist_new: failed to allocate files");

   p->capacity = PLAYLIST_CHUNK_SIZE;
   p->filename = NULL;
   p->name     = NULL;
   p->nfiles   = 0;
   p->history  = playlist_history_new();
   p->hist_present = -1;
   p->needs_saving = false;

   return p;
}

/* Free all of the memory consumed by a given playlist. */
void
playlist_free(playlist *p)
{
   if (p->filename != NULL) free(p->filename);
   if (p->name != NULL) free(p->name);
   if (p->files != NULL) free(p->files);
   playlist_history_free(p);
   free(p);
}

/*
 * Duplicate an existing playlist, returning a pointer to the newly allocated
 * playlist.  The filename and name of the new playlist must be specified.
 */
playlist *
playlist_dup(const playlist *original, const char *filename,
   const char *name)
{
   playlist *newplist;
   int i;

   /* create new playlist and copy simple members */
   newplist           = playlist_new();
   newplist->nfiles   = original->nfiles;
   newplist->capacity = original->nfiles;

   if (name != NULL) {
      if ((newplist->name = strdup(name)) == NULL)
         err(1, "playlist_dup: strdup name failed");
   }
   if (filename != NULL) {
      if ((newplist->filename = strdup(filename)) == NULL)
         err(1, "playlist_dup: strdup filename failed");
   }

   /* copy all of the files */
   newplist->files = calloc(original->nfiles, sizeof(meta_info*));
   if (newplist->files == NULL)
      err(1, "playlist_dup: failed to allocate files");

   for (i = 0; i < original->nfiles; i++)
      newplist->files[i] = original->files[i];

   return newplist;
}

/*
 * Add files to a playlist at the index specified by start.  Note that if
 * start is the length of the files array the files are appended to the end.
 */
void
playlist_files_add(playlist *p, meta_info **f, int start, int size, bool record)
{
   playlist_changeset *changes;
   int i;

   if (start < 0 || start > p->nfiles)
      errx(1, "playlist_file_add: index %d out of range", start);

   while (p->capacity <= p->nfiles + size)
      playlist_increase_capacity(p);

   /* push everything after start back size places */
   for (i = p->nfiles + size; i > start; i--)
      p->files[i] = p->files[i - size];

   /* add the files */
   for (i = 0; i < size; i++)
      p->files[start + i] = f[i];

   p->nfiles += size;

   /* update the history for this playlist */
   if (record) {
      changes = changeset_create(CHANGE_ADD, size, f, start);
      playlist_history_push(p, changes);
      p->needs_saving = true;
   }
}

/* Append a file to the end of a playlist */
void
playlist_files_append(playlist *p, meta_info **f, int size, bool record)
{
   return playlist_files_add(p, f, p->nfiles, size, record);
}

/* Remove a file at a given index from a playlist. */
void
playlist_files_remove(playlist *p, int start, int size, bool record)
{
   playlist_changeset *changes;
   int i;

   if (start < 0 || start >= p->nfiles)
      errx(1, "playlist_remove_file: index %d out of range", start);

   if (record) {
      changes = changeset_create(CHANGE_REMOVE, size, &(p->files[start]), start);
      playlist_history_push(p, changes);
      p->needs_saving = true;
   }

   for (i = start; i < p->nfiles; i++)
      p->files[i] = p->files[i + size];

   p->nfiles -= size;

}

/* Replaces the file at a given index in a playlist with a new file */
void
playlist_file_replace(playlist *p, int index, meta_info *newEntry)
{
   if (index < 0 || index >= p->nfiles)
      errx(1, "playlist_file_replace: index %d out of range", index);

   p->files[index] = newEntry;
}

/*
 * Loads a playlist from the provided filename.  The files within the playlist
 * are compared against the given meta-information-database to see if they
 * exist there.  If they do, the corresponding entry in the playlist structure
 * built is simply a pointer to the existing entry.  Otherwise, that file's
 * meta information is set to NULL and the file is copied (allocated) in the
 * playlist structure.
 *
 * A newly allocated playlist is returned.
 */
playlist *
playlist_load(const char *filename, meta_info **db, int ndb)
{
   meta_info *mi;
   FILE *fin;
   char *period;
   char  entry[PATH_MAX + 1];
   int   idx;

   /* open file */
   if ((fin = fopen(filename, "r")) == NULL)
      err(1, "playlist_load: failed to open playlist '%s'", filename);

   /* create playlist and setup */
   playlist *p = playlist_new();
   p->filename = strdup(filename);
   p->name     = strdup(basename(filename));
   if (p->filename == NULL || p->name == NULL)
      err(1, "playlist_load: failed to allocate info for playlist '%s'", filename);

   /* hack to remove '.playlist' from name */
   period  = strrchr(p->name, '.');
   *period = '\0';

   /* read each line from the file and copy into playlist object */
   while (fgets(entry, PATH_MAX, fin) != NULL) {
      /* sanitize */
      entry[strcspn(entry, "\n")] = '\0';

      /* check if file exists in the meta info. db */
      mi = NULL;
      for (idx = 0; idx < ndb; idx++) {
         if (strcmp(entry, db[idx]->filename) == 0)
            mi = db[idx];
      }

      if (mi != NULL)   /* file DOES exist in DB */
         playlist_files_append(p, &mi, 1, false);
      else {            /* file does NOT exist in DB */
         /* create empty meta-info object with just the file name */
         mi = mi_new();
         mi->filename = strdup(entry);
         if (mi->filename == NULL)
            err(1, "playlist_load: failed to strdup filename");

         /* add new record to the db and link it to the playlist */
         playlist_files_append(p, &mi, 1, false);
         warnx("playlist \"%s\", file \"%s\" is NOT in media database (added for now)",
            p->name, entry);
      }
   }

   fclose(fin);
   return p;
}

/*
 * Save a playlist to file.  The filename used is whatever is in the
 * playlist.
 */
void
playlist_save(const playlist *p)
{
   FILE *fout;
   int   i;

   if ((fout = fopen(p->filename, "w")) == NULL)
      err(1, "playlist_save: failed to open playlist \"%s\"", p->filename);

   /* write each song to file */
   for (i = 0; i < p->nfiles; i++) {
      if (fprintf(fout, "%s\n", p->files[i]->filename) == -1)
         err(1, "playlist_save: failed to record playlist \"%s\"", p->filename);
   }

   fclose(fout);
}

/*
 * Deletes playlist from disk and destroy's the object, free()'ing all
 * memory.
 */
void
playlist_delete(playlist *p)
{
   /* delete file if the playlist is stored in a file */
   if (p->filename != NULL && unlink(p->filename) != 0)
      err(1, "playlist_delete: failed to delete playlist \"%s\"", p->filename);

   /* destroy/free() all memory */
   playlist_free(p);
}

/*
 * Filter a playlist.  After a query string is setup using the meta_info
 * function "mi_query_set(..)" function, this function can be used to
 * filter out all of the entried of a playlist that match/do-not-match
 * that query string.
 *
 * The results are in the new playlist that is returned.
 * If no query is set with mi_query_init(), NULL is returned.
 *
 * The 'm' parameter controls if records matching should be returned
 * (m = true) or if records not matching should be returned (m=false)
 */
playlist *
playlist_filter(const playlist *p, bool m)
{
   playlist *results;
   int       i;

   if (!mi_query_isset())
      return NULL;
   
   results = playlist_new();
   for (i = 0; i < p->nfiles; i++) {
      if (mi_match(p->files[i])) {
         if (m)  playlist_files_append(results, &(p->files[i]), 1, false);
      } else {
         if (!m) playlist_files_append(results, &(p->files[i]), 1, false);
      }
   }

   return results;
}

/*
 * Builds an array of all files in the given directory with a '.playlist'
 * extension, returning the number of such files found.
 *
 * Parameters:
 *    dirname        C-string of directory containing playlist files
 *
 *    filenames      Pointer to an array of C-strings that will be allocated
 *                   and built in this function.  This array is where the
 *                   filename of each playlist will be stored.
 * Returns:
 *    The number of files with a '.playlist' extension that were found.
 *
 * Notes:
 *    All allocation of the filenames array is handled here.  It is the
 *    responsibility of the caller to free()
 *       1. each element of that array
 *       2. the array itself.
 */
int
retrieve_playlist_filenames(const char *dirname, char ***fnames)
{
   char   *glob_pattern;
   int     fcount;
   glob_t  files;

   /* build the search pattern */
   if (asprintf(&glob_pattern, "%s/*.playlist", dirname) == -1)
      errx(1, "failed in building glob pattern");

   /* get the files */
   if (glob(glob_pattern, 0, NULL, &files) != 0 && errno != 0)
      err(1, "failed to glob playlists directory");

   /* allocate & copy each of the filenames found into the filenames array */
   if ((*fnames = calloc(files.gl_pathc, sizeof(char*))) == NULL)
      err(1, "failed to allocate playlist filenames array");

   for (fcount = 0; fcount < files.gl_pathc; fcount++) {
      if (asprintf(&((*fnames)[fcount]), "%s", files.gl_pathv[fcount]) == -1)
         errx(1, "failed to allocate filename for playlist");
   }

   /* cleanup */
   globfree(&files);
   free(glob_pattern);

   return fcount;
}

playlist_changeset*
changeset_create(short type, size_t size, meta_info **files, int loc)
{
   size_t i;

   playlist_changeset *c;

   if ((c = malloc(sizeof(playlist_changeset))) == NULL)
      err(1, "%s: malloc(3) failed", __FUNCTION__);

   if ((c->files = calloc(size, sizeof(meta_info*))) == NULL)
      err(1, "%s: calloc(3) failed", __FUNCTION__);

   c->type = type;
   c->size = size;
   c->location = loc;

   for (i = 0; i < size; i++)
      c->files[i] = files[i];

   return c;
}

void
changeset_free(playlist_changeset *c)
{
   free(c->files);
   free(c);
}

playlist_changeset**
playlist_history_new(void)
{
   playlist_changeset **h;
   int i;

   if ((h = calloc(history_size, sizeof(playlist_changeset*))) == NULL)
      err(1, "%s: calloc(3) failed", __FUNCTION__);

   for (i = 0; i < history_size; i++)
      h[i] = NULL;

   return h;
}

void
playlist_history_free_future(playlist *p)
{
   int i;

   for (i = p->hist_present + 1; i < history_size; i++) {
      if (p->history[i] != NULL) {
         changeset_free(p->history[i]);
         p->history[i] = NULL;
      }
   }
}

void
playlist_history_free(playlist *p)
{
   p->hist_present = 0;
   playlist_history_free_future(p);
}

void
playlist_history_push(playlist *p, playlist_changeset *c)
{
   int i;

   if (p->hist_present < history_size - 1)
      playlist_history_free_future(p);
   else {
      for (i = 0; i < history_size - 1; i++)
         p->history[i] = p->history[i + 1];

      p->hist_present--;
   }

   p->hist_present++;
   p->history[p->hist_present] = c;
}

/* returns 0 if successfull, 1 if there was no history to undo */
int
playlist_undo(playlist *p)
{
   playlist_changeset *c;

   if (p->hist_present == -1)
      return 1;

   c = p->history[p->hist_present];

   switch (c->type) {
   case CHANGE_ADD:
      playlist_files_remove(p, c->location, c->size, false);
      break;
   case CHANGE_REMOVE:
      playlist_files_add(p, c->files, c->location, c->size, false);
      break;
   default:
      errx(1, "%s: invalid change type", __FUNCTION__);
   }

   p->hist_present--;
   return 0;
}

/* returns 0 if successfull, 1 if there was no history to re-do */
int
playlist_redo(playlist *p)
{
   playlist_changeset *c;

   if (p->hist_present == history_size - 1
   ||  p->history[p->hist_present + 1] == NULL)
      return 1;

   c = p->history[p->hist_present + 1];

   switch (c->type) {
   case CHANGE_ADD:
      playlist_files_add(p, c->files, c->location, c->size, false);
      break;
   case CHANGE_REMOVE:
      playlist_files_remove(p, c->location, c->size, false);
      break;
   default:
      errx(1, "%s: invalid change type", __FUNCTION__);
   }

   p->hist_present++;
   return 0;
}
