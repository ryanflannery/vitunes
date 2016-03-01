include ../config.mk

# install locations (overridden in config.mk above)
PREFIX?=/usr/local
BINDIR?=$(PREFIX)/bin

# combine all dependencies (from config.mk ... taglib/gstreamer/etc)
CDEPS=$(TAGLIB_CFLAGS) $(GSTREAMER_CFLAGS)
LDEPS=$(TAGLIB_LIBS) $(GSTREAMER_LIBS)
ODEPS=$(GSTREAMER_OBJS)

# object files
LOCAL_OBJS=commands.o compat.o ecmd.o \
			  ecmd_add.o ecmd_addurl.o ecmd_check.o \
			  ecmd_flush.o ecmd_help.o ecmd_init.o \
			  ecmd_rmfile.o ecmd_tag.o ecmd_update.o \
			  keybindings.o medialib.o meta_info.o \
			  mplayer.o paint.o player.o exe_in_path.o \
			  playlist.o socket.o str2argv.o \
			  uinterface.o vitunes.o

OBJS=$(ODEPS) $(LOCAL_OBJS)

# build variables
CC		  ?= /usr/bin/cc
CFLAGS  += -c -std=c89 -Wall -Wextra -Wno-unused-value $(CDEBUG) $(CDEPS)
LIBS    += -lm -lncurses -lutil $(LDEPS)

# subdirectories with code (.PATH for BSD make, VPATH for GNU make)
.PATH:  compat ecommands player player/gstreamer player/mplayer util
VPATH = compat ecommands player player/gstreamer player/mplayer util

.PHONY: clean debug install uninstall test

.DEFAULT: vitunes

vitunes: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $<

clean:
	rm -f $(LOCAL_OBJS)
	rm -f vitunes vitunes.core vitunes-debug.log

debug:
	$(MAKE) CDEBUG="-DDEBUG -g"

install: vitunes
	install -c -m 0555 vitunes $(BINDIR)

uninstall:
	rm -f $(BINDIR)/vitunes

###

TESTOBJS=exe_in_path.t.o str2argv.t.o gtest.t.o

gtest: $(OBJS) $(TESTOBJS)
	$(CC) -o $@ exe_in_path.o str2argv.o $(TESTOBJS) -L/usr/local/lib -lgtest
	./gtest

.cc.o:
	$(CC) -c -I/usr/local/include $<

TESTCFLAGS=-I/usr/local/include
TESTLDFLAGS=-L/usr/local/lib -lgtest
test:
	$(CC) -c -o $@.o $(TESTCFLAGS) util/str2argv.t.cc
	$(CC) -o $@ $(TESTLDFLAGS) test.o str2argv.o
	./$@

etest:
	$(CC) -c -o $@.o $(TESTCFLAGS) util/exe_in_path.t.cc
	$(CC) -o $@ $(TESTLDFLAGS) $@.o exe_in_path.o
	./$@
