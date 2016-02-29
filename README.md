vitunes
=======

A no-fluff media player & indexer, for vi users.

See [website](http://vitunes.org) for more information about vitunes.

The rest of this README is for developers interested in tweaking vitunes.


Introduction
============
For the hacking sort, this document may help navigate the source code to
vitunes.  ...and myself, 18 months from now when I dive back into vitunes.

For list of current todo's, known bug's, or other stuff to work on, grep the
source for TODO's and FIXME's.  Note that "XXX" is used only for important
notes.  For longer stuff (big, long-term TODO's), there's an up-to-date list
at the bottom of this file.

Below you'll find 3 sections.
 1. Media-Library Structure: a quick overview of how the media is handled
 2. Code-Structure: Notable Globals.  Listing of the 3 key global structs
 3. Code-Structure: The Modules.  Overview of each of the 9 modules.

Care has also been taken to make all .h files briefly document the API
therein.  This should be maintained.

This should be enough for most to jump-into hacking on vitunes.

For debugging (which can be painful in a curses application), see debug.h
and what happens when you "make -DDEBUG".  Two macros are created, one to
log to a file (DFLOG) and one to log to the console (DCLOG).  The log
file (vitunes-debug.log) is opened in vitunes.c if "-DDEBUG".

Media Library Structure
=======================

Before hacking on vitunes, it's important to understand the following about
the overall structure.

*  vitunes works by maintaining a database of all known files in
         ~/.vitunes/vitunes.db
*  The database is initialized, updated, and stuff added to it via, commands
   like
         $ vitunes -e init
         $ vitunes -e update
         $ vitunes -e add ~/music  /path/to/more/music ...
         $ vitunes -e addurl "http://...."
   See "vitunes -e help" for more details.
*  If a media file is not in the database, vitunes cannot play it.
*  Once the database is setup, vitunes can be run normally as
         $ vitunes
*  Once vitunes is run normally, one can browse the library, filter,
   sort, search, copy/paste/delete/etc and create new playlists.  All
   playlists are stored in
         ~/.vitunes/playlists/
   by default.
*  The playlist files themselves contain nothing more than the absolute
   paths (via realpath(3)) to the media files contained in the database.
   There is no meta information in playlists.  All meta information is
   in the database.
*  media playback is done with either
   1. a fork()'d instance of mplayer [2], or
   2. gstreamer
*  extracting meta-information from media files is done with the TagLib
   library [1], used in mi_extract() (meta_info.*) for extraction, and
   in ecmd_tag() (vitunes.c) for tagging.

Code Structure: Notable Globals
===============================

vitunes maintains a few key global objects (structs) that are used
throughout much of the code.  They are:

   1. mdb         Maintains all information about the media library,
                  including the database (an array of meta_info*'s), all
                  playlists, and the filter results.
                  See medialib.h for more info.

   2. player      Maintains all information about the fork()'d child
                  process of that handles playback, including pipes to
                  read/write to the process, the currently playing playlist
                  & song.  See player.h for more info.

   3. player_status
                  Maintains record keeping about what the media player is
                  up to.  Is is playing, paused, and what position (in
                  seconds) into the current file is it?  See player.h for
                  more info.

   4. ui          Maintains all key information about the display and each
                  of the 4 windows.  Of key interest are the two
                  "scrollable-windows" (the library and playlist windows).
                  The struct maintains their vertical/horizontal scroll
                  offsets, and a few more things.
                  See uinterface.h for more info.

There are other globals, but few are "exported" in any of the .h files.
These include global structs in:

   *  meta_info.c       For maintaining the current display description,
                        sort description, and search/query filter.  With
                        the exception of the display description, none of
                        these are exported, and should not be.

   *  paint.c           For maintaining the colors used for painting all
                        of the text to curses.


Code Structure: The Modules
===========================

First, the basic configuration is all done in config.h.  Specifically,
   *  keybindings
   *  command-mode bindings
   *  a few other defaults (library width, path to mplayer, etc.)
is all done there.  See that for quick fixes.

After that, the source code for vitunes is broken into 9 modules, each
relatively small.  A brief description of each follows, starting with the
smallest.  Note the code-naming conventions and which ones export a
global object that is used throughout the other modules.

Tracing through the source code from "main" to keybindings is, at this
point, relatively straight-forward.


MODULE            DESCRIPTION
--------------    ----------------------------------------------------------
str2argv          Contains only 2 functions used elsewhere.  They are used
                  for converting a string (char*) to an 'int argc' and
                  'char *argv[]' pair, used for the command-mode function
                  bindings.  The second function is used to free() a
                  previously built argc/argv structure.


player            Contains all of the code to handle the child-process that
                  handles media playback.  Includes loading files for
                  playback, pausing, seeking, etc.

                  Naming Convention:   player_*
                  Global Variables Exported:
                     player        - Record keeping for the child process.
                     player_status - Record keeping about the playback
                                     itself (paused, position, etc.).


meta_info         Contains all of the code to extract the meta information
                  (artist name, album name, song title, track number, genre,
                  and year) from a media-file.  It makes uses of the
                  external TagLib library [1].  It contains a simple struct
                  used to represent all such info (meta_info).

                  In addition to this, the logic for sorting songs,
                  searching/filtering songs, and changing their display in
                  vitunes is contained here.

                  Naming Convention:   mi_*
                  Global Variables Exported:
                     mi_display  -  Description of the current display
                                    format.


playlist          Contains all of the code to represent and work with
                  playlists, which is a simple struct containing an array
                  of meta_info's.  Nothing more.

                  Naming Convention:   playlist_*


medialib          Contains all of the code to represent the media library,
                  which is the database of all known files and array of all
                  playlists.  Handles initializing, loading, updating, and
                  adding to the database.

                  Naming Convention:   medialib_*
                  Global Variables Exported:
                     mdb   - The global media-library struct, containing
                             the database of known files and playlists.


uinterface        A small abstraction layer on top of curses(3), mostly for
                  record keeping.  Provides the code to setup the 4 windows,
                  resizing, and hiding/unhiding the library wndow.

                  Naming Convention:   ui_*
                                       swindow_*
                  Global Variables Exported:
                     ui    - The global user interface object, which again
                             contains mostly record-keeping info.


paint             All major display stuff is in here.  Functions for
                  painting each of the 4 window, error/information
                  messages, and setting up colors.

                  Naming Convention:   paint_*
                  Global Variables Exported:
                     colors   - List of colors used for each colorable
                                object in vitunes.


keybindings       All of the keybinding code is located here, including the
                  code for the default keybindings, the core bind and
                  unbind code, and more.


commands          The run-time commands (things like ':set ...',
                  ':color ...', etc.) is all contained here.


e_commands        All code for each of the e-commands (vitunes -e CMD ...)
                  is here.  Used exclusively in vitunes.c.


vitunes           The "main" program.  In normal mode, does setup of media-
                  library, user interface, and main input loop.
                  Also contains all of the code for all of the e-commands
                  (things like "vitunes -e update").


---

Hope this helps anyone interested in hacking on vitunes (specifically me... 6
months from now)

-Ryan Flannery  <ryan.flannery@gmail.com>  2 Jan. 2011
                   (Checked for accuracy)  1 Jan. 2012


References:
   [1]   http://developer.kde.org/~wheeler/taglib.html
   [2]   http://www.mplayerhq.hu/


CURRENT LIST OF BIG "TODO"'s:

*  Fix sorting of meta_info's so that numeric fields are sorted by their
   numeric values rather than character representations. This would require
   some re-working.

*  Sorting of meta_info's: perhaps I could add some magic to ignore a starting
   "The" or "A" from the sorting?  I dunno about this... I kinda like it the
   way that it is.

*  Original idea was to make the media player used be more general, so that it
   wasn't specific to mplayer.  Still working on this.  Goal is to have it
   setup so that one could do "vitunes -m /path/to/player" and the player
   would communicate through stdin/stdout (using a pipe to vitunes) in a
   unified way.  That way, the player could be a simple shell script that
   would allow 1. using different players, 2. using different players for
   different media types, and 3. making vitunes even more general... e.g.
   could launch an image viewer for images, or an image viewer for images in
   the meta info of a song.  Dunno.  Still working the details of this out.
   Could provide a shell script "wrapper" around multiple players/viewers.

*  Update from using 8-color curses API to 256-API.
   -> complicated... wtf!?!?  I thought the [n]curses API was supposed to make
      this easy?

:q
