/*
 * Copyright (c) 2010, 2011, 2012 Ryan Flannery <ryan.flannery@gmail.com>
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

#include "e_commands.h"

const struct ecmd ECMD_PATH[] = { 
   { "init",      ecmd_init },
   { "add",       ecmd_add },
   { "addurl",    ecmd_addurl },
   { "check",     ecmd_check },
   { "rmfile",    ecmd_rmfile },
   { "rm",        ecmd_rmfile },
   { "update",    ecmd_update },
   { "flush",     ecmd_flush },
   { "tag",       ecmd_tag },
   { "help",      ecmd_help }
};
const int ECMD_PATH_SIZE = (sizeof(ECMD_PATH) / sizeof(struct ecmd));

int
ecmd_init(int argc, char *argv[])
{
   if (argc != 1)
      errx(1, "usage: -e %s", argv[0]);

   printf("Creating all necessary files and directories for vitunes...\n");
   medialib_setup_files(vitunes_dir, db_file, playlist_dir);

   printf("\nNow use 'vitunes -e add dir1 dir2 ...' to add files to vitunes.\n");
   return 0;
}

int
ecmd_update(int argc, char *argv[])
{
   char ch;
   bool force_update = false;
   bool show_skipped = false;

   /* parse options */
   optreset = 1;
   optind = 0;
   while ((ch = getopt(argc, argv, "fs")) != -1) {
      switch (ch) {
         case 'f':
            force_update = true;
            break;
         case 's':
            show_skipped = true;
            break;
         case 'h':
         case '?':
         default:
            errx(1, "usage: -e %s [-fs]", argv[0]);
      }
   }
   if (optind < argc)
      errx(1, "usage: -e %s [-fs]", argv[0]);

   printf("Loading existing database...\n");
   medialib_load(db_file, playlist_dir);

   printf("Updating existing database...\n");
   medialib_db_update(show_skipped, force_update);

   medialib_destroy();
   return 0;
}

int
ecmd_add(int argc, char *argv[])
{
   if (argc == 1)
      errx(1, "usage: -e %s path [...]", argv[0]);

   printf("Loading existing database...\n");
   medialib_load(db_file, playlist_dir);

   printf("Scanning directories for files to add to database...\n");
   medialib_db_scan_dirs(argv + 1);

   medialib_destroy();
   return 0;
}

int
ecmd_addurl(int argc, char *argv[])
{
   meta_info   *m;
   bool         found;
   int          found_idx;
   char         input[255];
   int          field, i;

   if (argc != 2)
      errx(1, "usage: -e %s URL|path", argv[0]);

   /* start new record, set filename */
   m = mi_new();
   m->is_url = true;
   if ((m->filename = strdup(argv[1])) == NULL)
      err(1, "%s: strdup failed (filename)", argv[0]);

   /* get fields from user */
   for (field = 0; field < MI_NUM_CINFO; field++) {

      printf("%10.10s: ", MI_CINFO_NAMES[field]);
      if (fgets(input, sizeof(input), stdin) == NULL) {
         warnx("Operation canceled. Database unchanged.");
         mi_free(m);
         return 0;
      }

      if (input[strlen(input) - 1] == '\n')
         input[strlen(input) - 1] = '\0';

      if ((m->cinfo[field] = strdup(input)) == NULL)
         err(1, "%s: strdup failed (field)", argv[0]);
   }

   /* load existing database and see if file/URL already exists */
   medialib_load(db_file, playlist_dir);

   /* does the URL already exist in the database? */
   found = false;
   found_idx = -1;
   for (i = 0; i < mdb.library->nfiles && !found; i++) {
      if (strcmp(m->filename, mdb.library->files[i]->filename) == 0) {
         found = true;
         found_idx = i;
      }
   }

   if (found) {
      printf("Warning: file/URL '%s' already in the database.\n", argv[0]);
      printf("Do you want to replace the existing record? [y/n] ");

      if (fgets(input, sizeof(input), stdin) == NULL
      || (strcasecmp(input, "yes\n") != 0 && strcasecmp(input, "y\n") != 0)) {
         warnx("Operation Canceled.  Database unchanged.");
         mi_free(m);
         medialib_destroy();
         return 0;
      }

      mi_sanitize(m);
      playlist_file_replace(mdb.library, found_idx, m);
   } else {
      mi_sanitize(m);
      playlist_files_append(mdb.library, &m, 1, false);
   }

   medialib_db_save(db_file);
   medialib_destroy();

   return 0;
}

int
ecmd_check(int argc, char *argv[])
{
   meta_info *mi;
   bool   show_raw, show_sanitized, show_database;
   bool   found;
   char   ch;
   char   realfile[PATH_MAX];
   char **files;
   int    nfiles;
   int    f, i;

   if (argc < 3)
      errx(1, "usage: -e %s [-drs] path [...]", argv[0]);

   /* parse options to see which version to show */
   show_raw = false;
   show_sanitized = false;
   show_database = false;
   optreset = 1;
   optind = 0;
   while ((ch = getopt(argc, argv, "rsd")) != -1) {
      switch (ch) {
         case 'r':
            show_raw = true;
            break;
         case 's':
            show_sanitized = true;
            break;
         case 'd':
            show_database = true;
            break;
         case 'h':
         case '?':
         default:
            errx(1, "usage: -e %s [-drs] path [...]", argv[0]);
      }
   }
   files = argv + optind;
   nfiles = argc - optind;

   if (!show_raw && !show_sanitized && !show_database)
      errx(1, "%s: must specify at least one of -r, -s, or -d", argv[0]);

   if (nfiles == 0)
      errx(1, "%s: must provide at least one file to check.", argv[0]);

   /* scan through files... */
   for (f = 0; f < nfiles; f++) {

      printf("Checking: '%s'\n", files[f]);

      /* show raw or sanitized information */
      if (show_raw || show_sanitized) {
         mi = mi_extract(files[f]);
         if (mi == NULL)
            warnx("Failed to extract any meta-information from '%s'", files[f]);
         else {
            /* show raw info */
            if (show_raw) {
               printf("\tThe RAW meta-information from the file is:\n");
               for (i = 0; i < MI_NUM_CINFO; i++)
                  printf("\t%10.10s: '%s'\n", MI_CINFO_NAMES[i], mi->cinfo[i]);
            }

            /* show sanitized info */
            if (show_sanitized) {
               mi_sanitize(mi);
               printf("\tThe SANITIZED meta-information from the file is:\n");
               for (i = 0; i < MI_NUM_CINFO; i++)
                  printf("\t%10.10s: '%s'\n", MI_CINFO_NAMES[i], mi->cinfo[i]);
            }
         }
      }

      /* check if it's in the database */
      if (show_database) {

         /* get absolute filename */
         if (realpath(files[f], realfile) == NULL) {
            warn("%s: realpath failed for %s: skipping", argv[0], files[f]);
            continue;
         }

         /* check if file is in database */
         medialib_load(db_file, playlist_dir);

         found = false;
         mi = NULL;
         for (i = 0; i < mdb.library->nfiles && !found; i++) {
            if (strcmp(realfile, mdb.library->files[i]->filename) == 0) {
               found = true;
               mi = mdb.library->files[i];
            }
         }

         if (!found)
            warnx("File '%s' does NOT exist in the database", files[f]);
         else {
            printf("\tThe meta-information in the DATABASE is:\n");
            for (i = 0; i < MI_NUM_CINFO; i++)
               printf("\t%10.10s: '%s'\n", MI_CINFO_NAMES[i], mi->cinfo[i]);
         }

         medialib_destroy();
      }

   }

   return 0;
}

int
ecmd_rmfile(int argc, char *argv[])
{
   char *filename;
   char  input[255];
   bool  forced;
   bool  found;
   int   found_idx;
   int   i;

   if (argc < 2 || argc > 3)
      errx(1, "usage: -e %s [-f] URL|path", argv[0]);

   /* get filename and if this remove is forced */
   forced = false;
   if (argc == 3) {
      if (strcmp(argv[1], "-f") != 0)
         errx(1, "usage: -e %s [-f] URL|path", argv[0]);
      else
         forced = true;

      filename = argv[2];
   } else
      filename = argv[1];


   /* load database and search for record */
   medialib_load(db_file, playlist_dir);
   found = false;
   found_idx = -1;
   for (i = 0; i < mdb.library->nfiles && !found; i++) {
      if (strcmp(filename, mdb.library->files[i]->filename) == 0) {
         found = true;
         found_idx = i;
      }
   }

   /* if not found then error */
   if (!found) {
      i = (forced ? 0 : 1);
      errx(i, "%s: %s: No such file or URL", argv[0], filename);
   }

   /* if not forced, prompt user if they are sure */
   if (!forced) {
      printf("Are you sure you want to delete '%s'? [y/n] ", filename);
      if (fgets(input, sizeof(input), stdin) == NULL
      || (strcasecmp(input, "yes\n") != 0 && strcasecmp(input, "y\n") != 0))
         errx(1, "%s: operation canceled.  Database unchanged.", argv[0]);
   }

   playlist_files_remove(mdb.library, found_idx, 1, false);
   medialib_db_save(db_file);
   medialib_destroy();
   return 0;
}

int
ecmd_flush(int argc, char *argv[])
{
   char ch;
   char *time_format = "%Y %m %d %H:%M:%S";

   optreset = 1;
   optind = 0;
   while ((ch = getopt(argc, argv, "t:")) != -1) {
      switch (ch) {
         case 't':
            if ((time_format = strdup(optarg)) == NULL)
               err(1, "%s: strdup of time_format failed", argv[0]);
            break;
         case '?':
         case 'h':
         default:
            errx(1, "usage: %s [-t format]", argv[0]);
      }
   }
   
   medialib_load(db_file, playlist_dir);
   medialib_db_flush(stdout, time_format);
   medialib_destroy();
   return 0;
}

int
ecmd_tag(int argc, char *argv[])
{
   TagLib_File *tag_file;
   TagLib_Tag  *tag;
   bool  set_artist  = false;
   bool  set_album   = false;
   bool  set_title   = false;
   bool  set_genre   = false;
   bool  set_track   = false;
   bool  set_year    = false;
   bool  set_comment = false;
   char *artist = NULL, *album = NULL, *title = NULL, *genre = NULL,
        *comment = NULL;
   const char *errstr = NULL;
   unsigned int track = 0, year = 0;
   char   ch;
   char **files;
   int nfiles, f;

   static struct option longopts[] = {
      { "artist",  required_argument, NULL, 'a' },
      { "album",   required_argument, NULL, 'A' },
      { "title",   required_argument, NULL, 't' },
      { "genre",   required_argument, NULL, 'g' },
      { "track",   required_argument, NULL, 'T' },
      { "year",    required_argument, NULL, 'y' },
      { "comment", required_argument, NULL, 'c' },
      { NULL,     0,                 NULL,  0  }
   };

   /* parse options and get list of files */
   optreset = 1;
   optind = 0;
   while ((ch = getopt_long_only(argc, argv, "a:A:t:g:T:y:c:", longopts, NULL)) != -1) {
      switch (ch) {
         case 'a':
            set_artist = true;
            if ((artist = strdup(optarg)) == NULL)
               err(1, "%s: strdup ARTIST failed", argv[0]);
            break;
         case 'A':
            set_album = true;
            if ((album = strdup(optarg)) == NULL)
               err(1, "%s: strdup ALBUM failed", argv[0]);
            break;
         case 't':
            set_title = true;
            if ((title = strdup(optarg)) == NULL)
               err(1, "%s: strdup TITLE failed", argv[0]);
            break;
         case 'g':
            set_genre = true;
            if ((genre = strdup(optarg)) == NULL)
               err(1, "%s: strdup GENRE failed", argv[0]);
            break;
         case 'T':
            set_track = true;
            track = (unsigned int) strtonum(optarg, 0, INT_MAX, &errstr);
            if (errstr != NULL)
               errx(1, "%s: invalid track '%s': %s", argv[0], optarg, errstr);
            break;
         case 'y':
            set_year = true;
            year = (unsigned int) strtonum(optarg, 0, INT_MAX, &errstr);
            if (errstr != NULL)
               errx(1, "%s: invalid year '%s': %s", argv[0], optarg, errstr);
            break;
         case 'c':
            set_comment = true;
            if ((comment = strdup(optarg)) == NULL)
               err(1, "%s: strdup COMMENT failed", argv[0]);
            break;
         case 'h':
         case '?':
         default:
            errx(1, "usage: see 'vitunes -e help tag'");
      }
   }
   files  = argv + optind;
   nfiles = argc - optind;

   /* be verbose, indicate what we're setting... */
   printf("Setting the following tags to all files:\n");
   if (set_artist) printf("%10.10s => '%s'\n", "artist", artist);
   if (set_album) printf("%10.10s => '%s'\n", "album", album);
   if (set_title) printf("%10.10s => '%s'\n", "title", title);
   if (set_genre) printf("%10.10s => '%s'\n", "genre", genre);
   if (set_track) printf("%10.10s => %u\n", "track", track);
   if (set_year) printf("%10.10s => %u\n", "year", year);
   if (set_comment) printf("%10.10s => '%s'\n", "comment", comment);

   if (!set_artist && !set_album && !set_title && !set_genre
   &&  !set_track && !set_year && !set_comment)
      errx(1, "%s: nothing to set!  See 'vitunes -e help tag'", argv[0]);

   if (nfiles == 0)
      errx(1, "%s: must provide at least one file to tag.", argv[0]);

   /* tag files ... */
   taglib_set_strings_unicode(false);
   for (f = 0; f < nfiles; f++) {
      printf("tagging: '%s'\n", files[f]);

      /* extract taglib stuff */
      if ((tag_file = taglib_file_new(files[f])) == NULL) {
         warnx("TagLib: failed to open file '%s': skipping.", files[f]);
         printf("  => Causes: format not supported by TagLib or format doesn't support tags\n");
         fflush(stdout);
         continue;
      }

      tag = taglib_file_tag(tag_file);

      /* apply changes */
      if (set_artist) taglib_tag_set_artist(tag, artist);
      if (set_album) taglib_tag_set_album(tag, album);
      if (set_title) taglib_tag_set_title(tag, title);
      if (set_genre) taglib_tag_set_genre(tag, genre);
      if (set_track) taglib_tag_set_track(tag, track);
      if (set_year) taglib_tag_set_year(tag, year);
      if (set_comment) taglib_tag_set_comment(tag, comment);


      /* save changes and cleanup */
      taglib_file_save(tag_file);
      taglib_tag_free_strings();
      taglib_file_free(tag_file);
   }

   return 0;
}

int
ecmd_help(int argc, char *argv[])
{
   char *man_args[3];

   if (argc != 2)
      errx(1, "usage: -e %s [command]", argv[0]);

   /* no help requested for a specific command, give a list of all e-cmds */
   if (argc == 1) {
      printf("\
The following is a list of e-commands supported by vitunes.\n\
Each command is executed by doing:\n\
   $ vitunes -e COMMAND [args ...]\n\
The complete manual for each can be obtained by doing:\n\
   $ vitunes -e help COMMAND\n\
The list of available commands are:\n\n\
   Command     Description\n\
   -------     ------------------------------------------------------------\n\
   init        Create the initial database used by vitunes.\n\
   update      Scan all files in the database and update their records.\n\
   add         Add files to the vitunes database.\n\
   addurl      Add non-files/URLs to the vitunes database.\n\
   check       Scan files for meta-info and if they are in the database.\n\
   rm          Remove files/URLs from the database.\n\
   rmfile      Alias for \"rm\".\n\
   tag         Add or modify meta-information tags in raw files.\n\
   flush       Output the records of all files in the database.\n\
   help        This command.\n\
");
      return 0;
   }

   /* if reach here, help fora specific command was requested */
   if (strcmp(argv[1], "help") == 0) {
      printf("You're a damn fool if you need help with help.\n");
      return 0;
   }

   man_args[0] = "man";
   if (asprintf(&man_args[1], "vitunes-%s", argv[1]) == -1)
      err(1, "ecmd_help: asprintf failed");
   man_args[2] = NULL;

   execvp("man", man_args);
   err(1, "ecmd_help: execvp failed");

   /* just to shut up gcc */
   return 0;
}

int
ecmd_execute(int argc, char **argv, const char *ecmd)
{
   int i;

   for (i = 0; i < ECMD_PATH_SIZE; i++) {
      if (strcmp(ecmd, ECMD_PATH[i].name) == 0) {
         ECMD_PATH[i].func(argc, argv);
         return 0;
      }
   }

   return -1;
}
