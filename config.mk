### This is the default Makefile configuration for vitunes.
### The contents of this file are normally set by the configure.sh script.
### If you are seeing this text, then that script has *not* been run.
###
### This file controls:
###   1. Where vitunes is installed (see PREFIX, BINDER, and MANDIR below)
###   2. What capability vitunes will be compiled with (e.g. will gstreamer
###		support be included?  what will be the default media backend?)
###
### The configure.sh script, when run, will replace the contents of this
### file with what it detects to be most appropriate, based on your
### operating system and what is installed.
###
### The default contents of this file may not be suited to your OS & setup.
### Please run the configure.sh script.


#######################################################################
### Install Locations.  These control where vitunes is installed.
### Defaults are fine for Open/Free/Net BSD.
#######################################################################

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/man/man1


#######################################################################
### TagLib configuration.  TagLib is required to build and run vitunes.
### Defaults should be fine as long as TagLib is installed.
#######################################################################
TAGLIB_CFLAGS  = `pkg-config taglib --cflags`
TAGLIB_LDFLAGS = `pkg-config taglib --libs` -ltag_c


#######################################################################
### Defaults should be fine as long as gstreamer is installed.
#######################################################################

GSTREAMER_CFLAGS  = `pkg-config gstreamer-0.10 --cflags` -DENABLE_GSTREAMER
GSTREAMER_LDFLAGS = `pkg-config gstreamer-0.10 --libs`   -ltag_c
GSTREAMER_OBJS    = gstplayer.o


### eof
