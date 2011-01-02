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

#include "vitunes.h"
#include "config.h"     /* NOTE: must be after vitunes.h */


/*****************************************************************************
 * GLOBALS, EXPORTED
 ****************************************************************************/

/* key playlists to keep track of */
playlist *viewing_playlist;
playlist *playing_playlist;

/* signal flags */
volatile sig_atomic_t VSIG_QUIT = 0;            /* 1 = quit vitunes */
volatile sig_atomic_t VSIG_RESIZE = 0;          /* 1 = resize display */
volatile sig_atomic_t VSIG_PLAYER_MONITOR = 0;  /* 1 = update player stats */
volatile sig_atomic_t VSIG_PLAYER_RESTART = 0;  /* 1 = restart player */

/*
 * enum used for QUIT_CAUSE values. Currently only one is used, but might add
 * more.  see the end of the main function and the signal_handler for how/why
 * this is used.
 */
enum { EXIT_NORMAL, BAD_PLAYER };
volatile sig_atomic_t QUIT_CAUSE = EXIT_NORMAL;

/* used with -DDEBUG */
FILE  *debug_log;


/*****************************************************************************
 * GLOBALS, LOCAL
 ****************************************************************************/

/* location patterns of various things (%s is user's home directory) */
const char *VITUNES_DIR_FMT  = "%s/.vitunes";
const char *CONF_FILE_FMT    = "%s/.vitunes/vitunes.conf";
const char *DB_FILE_FMT      = "%s/.vitunes/vitunes.db";
const char *PLAYLIST_DIR_FMT = "%s/.vitunes/playlists";

/* absolute paths of key stuff */
char *vitunes_dir;
char *conf_file;
char *db_file;
char *playlist_dir;
char *player_path;


/*****************************************************************************
 * local functions
 ****************************************************************************/

/* misc. functions */
int  handle_switches(int argc, char *argv[]);
void usage(const char *);
void signal_handler(int);
void setup_timer();


int
main(int argc, char *argv[])
{
   struct passwd *pwd;
   int    previous_command;
   int    input;

#ifdef DEBUG
   if ((debug_log = fopen("vitunes-debug.log", "w")) == NULL)
      err(1, "failed to open debug log");
#endif

   /*------------------------------------------------------------------------
    * build paths names needed by vitunes & handle switches
    *----------------------------------------------------------------------*/

   /* get home dir */
   if ((pwd = getpwuid(getuid())) == NULL)
      errx(1, "invalid user %d", getuid());

   /* build paths */
   asprintf(&vitunes_dir,   VITUNES_DIR_FMT,  pwd->pw_dir);
   asprintf(&conf_file,     CONF_FILE_FMT,    pwd->pw_dir);
   asprintf(&db_file,       DB_FILE_FMT,      pwd->pw_dir);
   asprintf(&playlist_dir,  PLAYLIST_DIR_FMT, pwd->pw_dir);
   if (vitunes_dir == NULL || conf_file == NULL
   ||  db_file == NULL     || playlist_dir == NULL)
      err(1, "failed to create needed file names");

   player_path = DEFAULT_PLAYER_PATH;

   /* handle command-line switches & e-commands */
   handle_switches(argc, argv);

   /*
    * IF we've reached here, then there were no e-commands.
    * start vitunes normally...
    */


   /*------------------------------------------------------------------------
    * initialize stuff
    *--------------------------------------------------------------------- */

   /* setup signal handlers (XXX must be before player_child_launch) */
   signal(SIGPIPE,  SIG_IGN);          /* broken pipe with child (ignore) */
   signal(SIGCHLD,  signal_handler);   /* child died */
   signal(SIGHUP,   signal_handler);   /* quit */
   signal(SIGINT,   signal_handler);   /* quit */
   signal(SIGQUIT,  signal_handler);   /* quit */
   signal(SIGTERM,  signal_handler);   /* quit */
   signal(SIGWINCH, signal_handler);   /* resize */
   setup_timer();                      /* periodic timer to update player */

   /* init small stuff (XXX some must be done before medialib_load) */
   mi_query_init();        /* global query description */
   mi_sort_init();         /* global sort description */
   mi_display_init();      /* global display description */
   ybuffer_init();         /* global yank/copy buffer */

   /* load media library (database and all playlists) & sort */
   medialib_load(db_file, playlist_dir);
   if (mdb.library->nfiles == 0) {
      printf("The vitunes database is currently empty.\n");
      printf("See 'vitunes -e help add' for how to add files.");
      player_child_kill();
      return 0;
   }
   /* apply default sort to library */
   qsort(mdb.library->files, mdb.library->nfiles, sizeof(meta_info*), mi_compare);

   /* setup user interface and default colors */
   kb_init();
   ui_init(DEFAULT_LIBRARY_WINDOW_WIDTH);
   paint_setup_colors();

   /* basic ui setup to get ui started */
   setup_viewing_playlist(mdb.library);
   ui.library->nrows  = mdb.nplaylists;
   playing_playlist = NULL;

   /* start media player child */
   player_init(player_path, DEFAULT_PLAYER_ARGS, DEFAULT_PLAYER_MODE);
   player_child_launch();

   /* load config file and run commands in it now */
   load_config();

   /* initial painting of the display */
   paint_all();

   /* -----------------------------------------------------------------------
    * begin input loop
    * -------------------------------------------------------------------- */

   previous_command = -1;
   while (!VSIG_QUIT) {

      /* handle any signal flags */
      process_signals(true);

      /* handle any available input */
      if ((input = getch()) && input != ERR) {
         if (isdigit(input) &&  (input != '0' || gnum_get() > 0))
            gnum_add(input - '0');
         else if (input == '\n' && gnum_get() > 0 && previous_command >= 0)
            kb_execute(previous_command);
         else
            kb_execute(input);
      }
   }

   /* -----------------------------------------------------------------------
    * cleanup
    * -------------------------------------------------------------------- */

   ui_destroy();
   player_child_kill();
   medialib_destroy();

   mi_query_clear();
   ybuffer_free();

   /* do we have any odd cause for quitting? */
   if (QUIT_CAUSE != EXIT_NORMAL) {
      switch (QUIT_CAUSE) {
         case BAD_PLAYER:
            warnx("media player '%s' is not executing", player.program);
            break;
      }
   }

   return 0;
}

/* print proper usage */
void
usage(const char *pname)
{
   fprintf(stderr,"\
usage: %s [-f config-file] [-d database-file] [-p playlist-dir] [-m player-path] [-e COMMAND ...]\n\
See \"%s -e help\" for information about what e-commands are available.\n\
",
   pname, pname);
   exit(1);
}

/* actual signal handler */
void
signal_handler(int sig)
{
   switch (sig) {
      case SIGHUP:
      case SIGINT:
      case SIGQUIT:
      case SIGTERM:
         VSIG_QUIT = 1;
         break;
      case SIGALRM:
         VSIG_PLAYER_MONITOR = 1;
         break;
      case SIGWINCH:
         VSIG_RESIZE = 1;
         break;
      case SIGCHLD:
         VSIG_PLAYER_RESTART = 1;
         break;
   }
}

/* handle any signal flags */
void
process_signals(bool do_paint_message)
{
   static playlist *prev_queue = NULL;
   static int       prev_qidx = -1;
   static bool      prev_is_playing = false;
   static time_t    last_sigchld = -1;

   /* handle resize event */
   if (VSIG_RESIZE) {
      ui_resize();
      ui_clear();
      paint_all();
      VSIG_RESIZE = 0;
   }

   /* monitor player */
   if (VSIG_PLAYER_MONITOR) {
      player_monitor();

      if (prev_is_playing || player_status.playing)
         paint_player();

      /* need to repaint anything else? */
      if (prev_is_playing != player_status.playing) {
         paint_library();
         paint_playlist();
      } else if (prev_queue != player.queue) {
         paint_library();
         if (prev_queue == viewing_playlist) {
            paint_playlist();
         }
      }
      if (player.queue == viewing_playlist && prev_qidx != player.qidx) {
         paint_playlist();
      }

      prev_queue = player.queue;
      prev_qidx = player.qidx;
      prev_is_playing = player_status.playing;
      VSIG_PLAYER_MONITOR = 0;
   }

   /* restart player if needed */
   if (VSIG_PLAYER_RESTART) {
      /* ignore children launched from a '!' */
      if (kill(player.pid, 0) != 0) {
         /* if too frequent, quit */
         if (time(0) - last_sigchld <= 1) {
            QUIT_CAUSE = BAD_PLAYER;
            VSIG_QUIT = 1;
         } else {
            player_child_relaunch();
            if (do_paint_message)
               paint_message("child player died. re-launched.");
         }
      }
      VSIG_PLAYER_RESTART = 0;
   }
}

/* setup timer signal handler above */
void
setup_timer()
{
   struct sigaction sig_act;
   struct itimerval timer;

   /* create timer signal handler */
   if (sigemptyset(&sig_act.sa_mask) < 0)
      err(1, "setup_timer: sigemptyset failed");

   sig_act.sa_flags = 0;
   sig_act.sa_handler = signal_handler;
   if (sigaction(SIGALRM, &sig_act, NULL) < 0)
      err(1, "setup_timer: sigaction failed");

   /* setup timer details */
   timer.it_value.tv_sec = 0;
   timer.it_value.tv_usec = 500000;    /* 1 half-second */
   timer.it_interval.tv_sec = 0;
   timer.it_interval.tv_usec = 500000; /* 1 half-second */

   /* register timer */
   if (setitimer(ITIMER_REAL, &timer, NULL) < 0)
      err(1, "setup_timer: setitimer failed");
}

/*
 * load config file and execute all command-mode commands within.
 * XXX note that this requires mdb, ui, and player to all be loaded/setup
 */
void
load_config()
{
   const char *errmsg = NULL;
   size_t  length, linenum;
   FILE   *fin;
   char   *line;
   char   *copy;
   char  **argv;
   int     argc;
   bool    found;
   int     found_idx = 0;
   int     num_matches;
   int     i, ret;

   if ((fin = fopen(conf_file, "r")) == NULL)
      return;

   linenum = 0;
   while (!feof(fin)) {

      /* get next line */
      if ((line = fparseln(fin, &length, &linenum, NULL, 0)) == NULL) {
         if (ferror(fin))
            err(1, "error reading config file '%s'", conf_file);
         else
            break;
      }

      /* skip whitespace */
      copy = line;
      copy += strspn(copy, " \t\n");
      if (copy[0] == '\0') {
         free(line);
         continue;
      }

      /* parse line into argc/argv */
      if (str2argv(copy, &argc, &argv, &errmsg) != 0) {
         endwin();
         errx(1, "%s line %zd: parse error: %s", conf_file, linenum, errmsg);
      }

      /* run command */
      found = false;
      num_matches = 0;
      for (i = 0; i < CommandPathSize; i++) {
         if (match_command_name(argv[0], CommandPath[i].name)) {
            found = true;
            found_idx = i;
            num_matches++;
         }
      }

      if (found && num_matches == 1) {
         if ((ret = (CommandPath[found_idx].func)(argc, argv)) != 0) {
            endwin();
            errx(1, "%s line %zd: error with command '%s' [%i]",
               conf_file, linenum, argv[0], ret);
         }
      } else if (num_matches > 1) {
         endwin();
         errx(1, "%s line %zd: ambiguous abbreviation '%s'",
            conf_file, linenum, argv[0]);
      } else {
         endwin();
         errx(1, "%s line %zd: unknown command'%s'",
            conf_file, linenum, argv[0]);
      }

      argv_free(&argc, &argv);
      free(line);
   }

   fclose(fin);
}

/*
 * parse the command line and handle all switches.
 * this also handles all of the e-commands.
 */
int
handle_switches(int argc, char *argv[])
{
   int ch;
   int i;

   while ((ch = getopt(argc, argv, "he:f:d:p:m:")) != -1) {
      switch (ch) {
         case 'd':
            if ((db_file = strdup(optarg)) == NULL)
               err(1, "handle_switches: strdup db_file failed");
            break;

         case 'e':
            /* an e-command */
            argc -= optind - 1;
            argv += optind - 1;

            for (i = 0; i < ECMD_PATH_SIZE; i++) {
               if (strcmp(optarg, ECMD_PATH[i].name) == 0)
                  exit((ECMD_PATH[i].func)(argc, argv));
            }

            errx(1, "Unknown e-command '%s'.  See 'vitunes -e help' for list.",
               optarg);
            break;

         case 'f':
            if ((conf_file = strdup(optarg)) == NULL)
               err(1, "handle_switches: strdup conf_file failed");
            break;

         case 'm':
            if ((player_path = strdup(optarg)) == NULL)
               err(1, "handle_switches: strdup player_path failed");
            break;

         case 'p':
            if ((playlist_dir = strdup(optarg)) == NULL)
               err(1, "handle_switches: strdup playlist_dir failed");
            break;

         case 'h':
         case '?':
         default:
            usage(argv[0]);
            /* NOT REACHED */
      }
   }

   return 0;
}
