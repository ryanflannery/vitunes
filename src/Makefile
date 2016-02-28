include ../config.mk

# install locations (overridden in config.mk above)
PREFIX?=/usr/local
BINDIR?=$(PREFIX)/bin
MANDIR?=$(PREFIX)/man/man1

# combine all dependencies (from taglib & config.mk)
CDEPS=$(TAGLIB_CFLAGS)  $(GSTREAMER_CFLAGS)
LDEPS=$(TAGLIB_LIBS) $(GSTREAMER_LIBS)
ODEPS=$(GSTREAMER_OBJS)

# build variables
CC		  ?= /usr/bin/cc
CFLAGS  += -c -std=c89 -Wall -Wextra -Wno-unused-value $(CDEBUG) $(CDEPS)
LIBS    += -lm -lncurses -lutil $(LDEPS)

# object files
OBJS=commands.o compat.o ecmd.o \
	  ecmd_add.o ecmd_addurl.o ecmd_check.o \
	  ecmd_flush.o ecmd_help.o ecmd_init.o \
	  ecmd_rmfile.o ecmd_tag.o ecmd_update.o \
	  keybindings.o medialib.o meta_info.o \
	  mplayer.o paint.o player.o player_utils.o \
	  playlist.o socket.o str2argv.o \
	  uinterface.o vitunes.o \
	  $(ODEPS)

# subdirectories with code (.PATH for BSD make, VPATH for GNU make)
.PATH:  players
VPATH = players

.PHONY: debug clean clean-all install uninstall

# main build targets
.DEFAULT: vitunes

vitunes: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $<

debug:
	make CDEBUG="-DDEBUG -g"

clean:
	rm -f *.o
	rm -f vitunes vitunes.core vitunes-debug.log

clean-all: clean
	rm -rf report.*

install: vitunes
	mkdir -p $(MANDIR)
	install -c -m 0555 vitunes $(BINDIR)
	install -c -m 0444 man/vitunes*.1 $(MANDIR)

uninstall:
	rm -f $(BINDIR)/vitunes
	rm -f $(MANDIR)/vitunes*.1

### static analysis checks

flawfinder: *.c
	flawfinder *.c

report.cppcheck: *.h *.c
	@figlet "cppcheck"
	cppcheck --std=c89 --enable=all --inline-suppr  -i compat . 2> $@
	@cat $@

report.scan-build: *.h *.c
	@figlet "clang analyzer"
	make clean
	mkdir -p $@
	scan-build -o $@ make

### wrapper for static checks above
.PHONY: reports
reports: report.mandoc report.cppcheck report.scan-build
	@figlet "Static Checks Complete"
	cat report.mandoc
	cat report.cppcheck

