include config.mk

# install locations (these are normally overriden by config.mk & configure.sh)
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/man/man1

# non-base dependencies (TagLib)
TAGLIB_CFLAGS  ?= `taglib-config --cflags`
TAGLIB_LDFLAGS ?= `taglib-config --libs` -ltag_c

# combine all dependencies (from taglib & config.mk)
CFLAGS_DEPS  = $(TAGLIB_CFLAGS)  $(GSTREAMER_CFLAGS)
LDFLAGS_DEPS = $(TAGLIB_LDFLAGS) $(GSTREAMER_LDFLAGS)
OBJ_DEPS     = $(GSTREAMER_OBJS)

# build variables
CC		  ?= /usr/bin/cc
CFLAGS  += -c -std=c89 -Wall -Wextra -Wno-unused-value $(CDEBUG) $(CFLAGS_DEPS)
LDFLAGS += -lm -lncursesw -lutil $(LDFLAGS_DEPS)

# object files
OBJS=commands.o compat.o e_commands.o \
	  keybindings.o medialib.o meta_info.o \
	  mplayer.o paint.o player.o player_utils.o \
	  playlist.o socket.o str2argv.o \
	  uinterface.o vitunes.o \
	  $(OBJ_DEPS)

# subdirectories with code (.PATH for BSD make, VPATH for GNU make)
.PATH:  players
VPATH = players


.PHONY: debug clean install uninstall man-debug

# main build targets

vitunes: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $<

debug:
	make CDEBUG="-DDEBUG -g"

clean:
	rm -f *.o
	rm -f vitunes vitunes.core vitunes-debug.log

install: vitunes
	install -c -m 0555 vitunes $(BINDIR)
	install -c -m 0444 doc/vitunes*.1 $(MANDIR)

uninstall:
	rm -f $(BINDIR)/vitunes
	rm -f $(MANDIR)/vitunes*.1

# misc. targets

man-debug:
	-mandoc -Wall doc/vitunes.1 > /dev/null
	-mandoc -Wall doc/vitunes-add.1 > /dev/null
	-mandoc -Wall doc/vitunes-addurl.1 > /dev/null
	-mandoc -Wall doc/vitunes-check.1 > /dev/null
	-mandoc -Wall doc/vitunes-flush.1 > /dev/null
	-mandoc -Wall doc/vitunes-init.1 > /dev/null
	-mandoc -Wall doc/vitunes-rm.1 > /dev/null
	-mandoc -Wall doc/vitunes-tag.1 > /dev/null
	-mandoc -Wall doc/vitunes-update.1 > /dev/null

vitunes.html: vitunes.1
	man2web vitunes > vitunes.html

cscope.out: *.h *.c
	cscope -bke

