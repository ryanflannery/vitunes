/*
 * Copyright (c) 2010 Ryan Flannery <ryan.flannery@gmail.com>
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
   { "init",      ecmd_init,     ecmd_help_init},
   { "add",       ecmd_add,      ecmd_help_add },
   { "addurl",    ecmd_addurl,   ecmd_help_addurl },
   { "check",     ecmd_check,    ecmd_help_check },
   { "rmfile",    ecmd_rmfile,   ecmd_help_rmfile },
   { "rm",        ecmd_rmfile,   ecmd_help_rmfile },
   { "update",    ecmd_update,   ecmd_help_update },
   { "flush",     ecmd_flush,    ecmd_help_flush },
   { "tag",       ecmd_tag,      ecmd_help_tag },
   { "help",      ecmd_help,     NULL }
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
   bool show_skipped = false;

   if (argc == 2 && strcmp(argv[1], "-s") == 0)
      show_skipped = true;

   if (argc != 1 && !show_skipped)
      errx(1, "usage: -e %s [-s]", argv[0]);

   printf("Loading existing database...\n");
   medialib_load(db_file, playlist_dir);

   printf("Updating existing database...\n");
   medialib_db_update(show_skipped);

   medialib_destroy();
   return 0;
}

int
ecmd_add(int argc, char *argv[])
{
   if (argc == 1)
      errx(1, "usage: -e %s /path/to/filesORdirs [ ... ] ", argv[0]);

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
      errx(1, "usage: -e %s filename|URL", argv[0]);

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
      playlist_file_append(mdb.library, m);
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
      errx(1, "usage: -e %s [-rsd] file1 [ file2 ... ]", argv[0]);

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
            errx(1, "usage: -e %s [-rsd] file1 [ file2 ... ]", argv[0]);
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
      errx(1, "usage: -e %s [-f] filename|URL", argv[0]);

   /* get filename and if this remove is forced */
   forced = false;
   if (argc == 3) {
      if (strcmp(argv[1], "-f") != 0)
         errx(1, "usage: -e %s [-f] filename|URL", argv[0]);
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

   playlist_file_remove(mdb.library, found_idx);
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
   if (set_track) printf("%10.10s => %i\n", "track", track);
   if (set_year) printf("%10.10s => %i\n", "year", year);
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

void
ecmd_help_init(void)
{
   printf("\
VITUNES COMMAND:\n\tinit - initialize vitunes directories/database\n\n\
SYNOPSIS:\n\tinit\n\n\
DESCRIPTION:\n\
   The init command is used to setup the initial files and directories\n\
   used by vitunes.  This includes:\n\n\
      ~/.vitunes              The vitunes core directory where everything\n\
                              is stored.\n\
      ~/.vitunes/vitunes.db   The library database containing the meta\n\
                              information of all media files.\n\
      ~/.vitunes/playlists/   Directory where all playlists are stored.\n\n\
   The database created is initially empty.\n\
   If any of the above files/directories exists, nothing will be changed.\n\
   This command takes no parameters.\n\n\
   It is not strictly necessary to run this command to start using\n\
   vitunes, as you could use an existing installation, or create the\n\
   files/directories yourself.  It is provided for convenience.\n\n\
EXAMPLE:\n\
      $ vitunes -e init\n\n\
");
}

void
ecmd_help_add(void)
{
   printf("\
VITUNES COMMAND:\n\tadd - add files to the vitunes database\n\n\
SYNOPSIS:\n\tadd /path/to/files1 [ path2 ... ]\n\n\
DESCRIPTION:\n\
   The add command is used to add files to the database used by vitunes.\n\
   For every file/directory provided as a parameter, vitunes will scan that\n\
   file or directory (recursively) searching for media files that contain\n\
   any meta information.  Any such files found are added to the database\n\
   used by vitunes.  If any of the files found are already in the database\n\
   then they will be re-scanned and any changes will be updated.\n\n\
   Note that vitunes only maintains information about the file, and not the\n\
   file itself.  It does NOT move/copy/modify the files in its database in\n\
   any way.\n\n\
   The information vitunes stores for each file includes:\n\
      *  Filename                *  Track Number\n\
      *  Artist Name             *  Year\n\
      *  Album Name              *  Genre\n\
      *  Song/Video Title        *  Play Length (seconds)\n\n\
   The filename stored is the absolute pathname obtained from realpath(3),\n\
   and serves as the key-field within the database.\n\n\
   If any file encountered has no meta information, it is NOT added to the\n\
   database.\n\n\
EXAMPLE:\n\
   $ vitunes -e add ~/music /usr/local/share/music\n\n\
");
}

void
ecmd_help_addurl(void)
{
   printf("\
VITUNES COMMAND:\n\taddurl - add a URL (or other) to the vitunes database\n\n\
SYNOPSIS:\n\taddurl URL\n\n\
DESCRIPTION:\n\
   To add non-standard-files to the vitunes database (things like URL's for\n\
   Internet radio streams), one can use the addurl command. It takes a single\n\
   parameter: the URL/filename to be added to the database. After that, you\n\
   will be prompted to enter meta-information for each field vitunes indexes\n\
   (artist, album, title, track, year, and genre). You may leave any/all of\n\
   the fields blank.\n\n\
   Basically, anything \"foo\" that mplayer can play by a simple:\n\
      $ mplayer foo\n\
   can be added to the database using this command.  Although regular files\n\
   could also be added using this command, the add command is preferred, as\n\
   it attempts to extract meta information automatically.\n\n\
   Note that files added to the database using the addurl command are NOT\n\
   checked for updates during an update command.\n\n\
   Note that the addurl command can also be used to change the meta-\n\
   information of an existing URL within the database.\n\n\
EXAMPLE:\n\
   $ vitunes -e addurl \"http://198.234.121.118:80\"\n\
   Artist: WVXU Online Radio<ENTER>\n\
    Album: Cincinnati Public Radio<ENTER>\n\
    Title: NPR<ENTER>\n\
    Track: <ENTER>\n\
     Year: <ENTER>\n\
    Genre: Radio<ENTER>\n\
   Length: INF<ENTER>\n\n\
NOTES:\n\
   When the vitunes database has to be re-built (because of say, an upgrade\n\
   where the database format has changed, or you simply deleted your\n\
   database), re-adding URL's can be tedious.  To ease this, consider using\n\
   a shell script such as the 'add_urls.sh' script found on the vitunes\n\
   website for storing & adding all of your URL's.  The script simply\n\
   executes the addurl command with all meta-information provided.  As an\n\
   example, the above EXAMPLE could be automated as:\n\n\
      #!/bin/sh\n\
      echo \"WVXU Online Radio\\n\\\n\
      Cincinnati Public Radio\\n\\\n\
      NPR\\n\\\n\
      \\n\\\n\
      \\n\\\n\
      Radio\\n\\\n\
      INF\\n\" | vitunes -e addurl \"http://198.234.121.118:80\"\n\n\
");
}

void
ecmd_help_check(void)
{
   printf("\
VITUNES COMMAND:\n\tcheck - check files for meta-info and if they're in the DB\n\n\
SYNOPSIS:\n\tcheck [-rsd] file1 [ file2 ... ]\n\n\
DESCRIPTION:\n\
   The check command will scan each filename provided to see to see if\n\
   vitunes can extract any meta-information, to see how vitunes \"sanitizes\"\n\
   the meta-information (see below), or what information vitunes currently has\n\
   in the database for this file.\n\n\
   Note that at least one of the -r, -s, or -d flags must be present.\n\n\
   The options are as follows:\n\n\
   -r    Show the raw information extracted directly from each file.\n\n\
   -s    Show the sanitized information after extracting it from each file.\n\
         See the section SANITATION below for details on what this is.\n\n\
   -d    Load the vitunes database and check if the each exists within it.\n\
         If so, show the information in the vitunes database.\n\n\
   If multiple files are provided, each will be checked in order.\n\n\
SANITATION:\n\
   By \"sanitation\" above, the following is meant: some media files have\n\
   non-printable characters in their meta information for one reason or\n\
   another (often, it's a faulty ripper/tagging application).  Sometimes\n\
   these non-printable characters can be control sequences used by [n]curses\n\
   and thus cause problems with the curses display of vitunes.  The way\n\
   vitunes sanitizes such data is to replace all such control characters with\n\
   an '?'.\n\n\
EXAMPLE:\n\
   $ vitunes -e check -r /path/to/file.mp3\n\n\
");
}

void
ecmd_help_rmfile(void)
{
   printf("\
VITUNES COMMAND:\n\trmfile - remove a single file/URL from the vitunes database\n\n\
SYNOPSIS:\n\trmfile [-f] filename/URL\n\n\
DESCRIPTION:\n\
   To remove a single file/URL from the vitunes database, the rmfile command\n\
   may be used.  It takes a single parameter: the filename/URL of the file to\n\
   remove.  Normally, you will be prompted if you are sure you want to\n\
   remove the file as a safety measure.  This prompt can be avoided using\n\
   the '-f' flag, to force the removal.\n\n\
   Note that to remove files, the full, absolute path to the file must be\n\
   provided, as obtained from realpath(3).\n\n\
EXAMPLE:\n\
   $ vitunes -e rmfile -f \"http://198.234.121.118/listen.pls\"\n\n\
");
}

void
ecmd_help_update(void)
{
   printf("\
VITUNES COMMAND:\n\tupdate - update vitunes database\n\n\
SYNOPSIS:\n\tupdate [-s]\n\n\
DESCRIPTION:\n\
   The update command loads the existing meta information database used\n\
   by vitunes and for each media file listed in the database, the file is\n\
   checked to see if it has been removed or modified since it was added\n\
   to the database.\n\n\
   If the file has been removed, it will be removed from the database.  If\n\
   the file has been modified, it's meta information will be extracted\n\
   again and the database will be updated.\n\
   Note that if there are errors while checking the file, the error will be\n\
   reported but the file, and its meta information, will remain in the\n\
   vitunes database.\n\n\
      -s       When present, files that are skipped because they have not\n\
               been modified will also be reported to stdout.  Normally,\n\
               only files that are updated are reported.\n\n\
   In short, anytime you remove/modify media files already in the vitunes\n\
   database, you should run this command.\n\n\
NOTE ABOUT URLS:\n\
   Note that files added to the database using the 'addurl' e-command will\n\
   NOT be checked/updated in any way, for obvious reasons.\n\n\
EXAMPLE:\n\
      $ vitunes -e update\n\n\
");
}

void
ecmd_help_flush(void)
{
   printf("\
VITUNES COMMAND:\n\tflush - dump output of vitunes database to terminal\n\n\
SYNOPSIS:\n\tflush [-t time-format]\n\n\
DESCRIPTION:\n\
   The flush command simply outputs the contents of the meta-information\n\
   database used by vitunes to stdout, in a fairly easy to read (but very\n\
   easy to parse/grep through) format.\n\n\
   The one optional parameter, time-format, can be any string acceptable\n\
   to strftime(3) and is used when displaying the 'last-updated' field of\n\
   each record in the database (when the file was last checked for meta-\n\
   information).\n\n\
   The format used is a simple comma-separated-value (CSV) one, where most\n\
   fields (any that can contain spaces/commas) are within double quotes.\n\
   The first line contains the field names, and those fields that are quoted\n\
   are also quoted in this header row.\n\n\
EXAMPLE:\n\
   $ vitunes -e flush\n\n\
   To see which files were last updated this month:\n\
   $ vitunes -e flush -t \"%%M %%Y\" | grep \"January 2010\"\n\n\
REFERENCES:\n\
   For some quick sed(1) one-liners on how to parse CSV data like the\n\
   output of this command, you can visit the following website:\n\
               http://sed.sourceforge.net/sedfaq4.html\n\n\
");
}

void
ecmd_help_tag(void)
{
   printf("\
VITUNES COMMAND:\n\ttag - set meta-information tags to raw files\n\n\
SYNOPSIS:\n\ttag [--artist=string] [-a string] [--album=string] [-A value]\n\
\t    [--title=string] [-t string] [--genre=string] [-g string]\n\
\t    [--track=number] [-T number] [--year=number] [-y number]\n\
\t    [--comment=string] [-c string]\n\
\t    file1 [ file2 ... ]\n\n\
DESCRIPTION:\n\
   The tag command is provided to add/change the meta-information tags of\n\
   media files.  The meta-information fields that can be set are: artist,\n\
   album, title, genre, track, and year.\n\n\
   Please note that this command only changes the meta-information in the\n\
   raw files themselves and NOT in the vitunes database.  To update the\n\
   vitunes database after tagging, use the 'update' e-command.\n\n\
   The tag command takes a series of field/value specifiers and any number\n\
   of files.  For each file specified, the given field will be set to the\n\
   provided value.\n\n\
   The options are as follows:\n\n\
   --artist=string\n\
   -a string            Sets the artist field to the provided string.\n\n\
   --album=string\n\
   -A string            Sets the album field to the provided string.\n\n\
   --title=string\n\
   -t string            Sets the title field to the provided string.\n\n\
   --genre=string\n\
   -g string            Sets the genre field to the provided string.\n\n\
   --comment=string\n\
   -c string            Sets the comment field to the provided string.\n\n\
   --track=number\n\
   -T number            Sets the track field to the provided number.\n\
                        Note that the number must be between 0 and\n\
                        INT_MAX.\n\n\
   --year=number\n\
   -y number            Sets the year field to the provided number.\n\
                        Note that the number must be between 0 and\n\
                        INT_MAX.\n\n\
   At least one tag option must be provided and at least one file must\n\
   be provided.\n\n\
IMPORTANT NOTE:\n\
   Just to reiterate a comment above, this command only changes the meta-\n\
   information in the raw files themselves and NOT in the vitunes database.\n\
   To update the vitunes database after tagging, use the 'update' or 'add'\n\
   e-commands.\n\n\
EXAMPLE:\n\
   CD rippers frequently pull information from CDDB (or other databases)\n\
   where, for example, a \"The\" is missing from an artist/album name\n\
   when it is, in fact, appropriate.  Below is an example of correcting\n\
   this and then updating the vitunes database:\n\n\
   $ vitunes -e tag --artist=\"The White Stripes\" /path/to/De_Stijl/*\n\
   $ vitunes -e update\n\n\
");
}

int
ecmd_help(int argc, char *argv[])
{
   bool found;
   int  i;

   if (argc > 2)
      errx(1, "usage: -e %s [command]", argv[0]);

   /* no help requested for a specific command, give a list of all e-cmds */
   if (argc == 1) {
      printf("\
The following is a list of e-commands supported by vitunes.\n\
Each command is executed by doing:\n\n\
   $ vitunes -e COMMAND [args ...]\n\n\
The complete manual for each can be obtained by doing:\n\n\
   $ vitunes -e help COMMAND\n\n\
The list of available commands are:\n\n\
   COMMAND     BRIEF DESCRIPTION\n\
   -------     ------------------------------------------------------------\n\
   init        Create the initial files used by vitunes.\n\n\
   update      Load the existing database and check all files contained\n\
               within to see they have been removed or modified.  The\n\
               library is updated accordingly.\n\n\
   add         Scan the list of provided files/directories for files to add\n\
               to the database used by vitunes.\n\n\
   addurl      Add a URL to the database, where you provide your own meta\n\
               information.\n\n\
   check       Check files to see what meta-information vitunes can extract,\n\
               sanitize, and whether or not it's in the database.\n\n\
   rmfile      Remove a file/URL from the database.\n\n\
   tag         Add/modify meta-information tags of raw files.\n\n\
   flush       Load the existing database and dump it's information in an\n\
               easy-to-parse format to stdout.\n\n\
   help        This command.\n\n\
");
      return 0;
   }

   /* if reach here, help fora specific command was requested */
   found = false;
   for (i = 0; i < ECMD_PATH_SIZE && !found; i++) {
      if (strcmp(argv[1], "help") == 0) {
         printf("You're a damn fool if you need help with help.\n");
         found = true;
      }
      else if (strcmp(argv[1], ECMD_PATH[i].name) == 0) {
         (ECMD_PATH[i].help)();
         found = true;
      }
   }

   if (!found)
      printf("Unknown command \"%s\".  See \"vitunes -e help\" for list.\n", argv[1]);

   return 0;
}

