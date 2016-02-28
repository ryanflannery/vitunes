include ../config.mk

# install locations (overridden in config.mk above)
PREFIX?=/usr/local
BINDIR?=$(PREFIX)/bin
MANDIR?=$(PREFIX)/man/man1

# combine all dependencies (from config.mk ... taglib/gstreamer/etc)
CDEPS=$(TAGLIB_CFLAGS)  $(GSTREAMER_CFLAGS)
LDEPS=$(TAGLIB_LIBS) $(GSTREAMER_LIBS)
ODEPS=$(GSTREAMER_OBJS)

# object files
LOCAL_OBJS=commands.o compat.o ecmd.o \
			  ecmd_add.o ecmd_addurl.o ecmd_check.o \
			  ecmd_flush.o ecmd_help.o ecmd_init.o \
			  ecmd_rmfile.o ecmd_tag.o ecmd_update.o \
			  keybindings.o medialib.o meta_info.o \
			  mplayer.o paint.o player.o player_utils.o \
			  playlist.o socket.o str2argv.o \
			  uinterface.o vitunes.o

OBJS=$(ODEPS) $(LOCAL_OBJS)

# build variables
CC		  ?= /usr/bin/cc
CFLAGS  += -c -std=c89 -Wall -Wextra -Wno-unused-value $(CDEBUG) $(CDEPS)
LIBS    += -lm -lncurses -lutil $(LDEPS)


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

clean:
	rm -f $(LOCAL_OBJS)
	rm -f vitunes vitunes.core vitunes-debug.log

install: vitunes
	install -c -m 0555 vitunes $(BINDIR)

uninstall:
	rm -f $(BINDIR)/vitunes

### static analysis checks

debug:
	$(MAKE) CDEBUG="-DDEBUG -g"

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
reports: report.cppcheck report.scan-build
	@figlet "Static Checks Complete"
	cat report.cppcheck

