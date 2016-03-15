include ../config.mk

# install locations (overridden in config.mk above)
PREFIX?=/usr/local
BINDIR?=$(PREFIX)/bin

# combine all dependencies (from config.mk ... taglib/gstreamer/etc)
CDEPS=$(TAGLIB_CFLAGS) $(GSTREAMER_CFLAGS)
LDEPS=$(TAGLIB_LIBS) $(GSTREAMER_LIBS)
ODEPS=$(GSTREAMER_OBJS)

# build variables
CC		  ?= clang
CFLAGS  += -c -std=c89 -Wall -Wextra -Wno-unused-value -g $(CDEPS)
LDFLAGS += -lm -lncurses -lutil -lpthread -lsqlite3 $(LDEPS)

# object files
OBJS=commands.o \
	  compat.o \
	  ecmd.o \
	  ecmd_add.o \
	  ecmd_addurl.o \
	  ecmd_check.o \
	  ecmd_flush.o \
	  ecmd_help.o \
	  ecmd_init.o \
	  ecmd_rmfile.o \
	  ecmd_tag.o \
	  ecmd_update.o \
	  exe_in_path.o \
	  keybindings.o \
	  medialib.o \
	  meta_info.o \
	  mplayer.o \
	  paint.o \
	  player.o \
	  playlist.o \
	  socket.o \
	  str2argv.o \
	  uinterface.o \
	  vitunes.o \
	  mfile.o \
	  mfile_taglib.o \
	  plist.o \
	  indent.o \
	  mediadb.o \
	  dparray.o

# subdirectories with code (.PATH for BSD make, VPATH for GNU make)
.PATH:  compat ecommands player player/gstreamer player/mplayer util mfile mfile/taglib plist mediadb
VPATH = compat ecommands player player/gstreamer player/mplayer util mfile mfile/taglib plist mediadb

.PHONY: clean debug install uninstall test

.DEFAULT: vitunes

vitunes: $(OBJS) $(ODEPS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(ODEPS)

.c.o:
	$(CC) $(CFLAGS) $<

clean:
	rm -f vitunes vitunes.core
	rm -f $(OBJS)
	rm -f vitunes-debug.log
	rm -f test test.core
	rm -f $(TEST_OBJS)

mediadb.o: schema.c

schema.c: schema.sql
	xxd -i schema.sql > $@

debug:
	$(MAKE) CDEBUG="-DDEBUG -g"

install: vitunes
	install -c -m 0555 vitunes $(BINDIR)

uninstall:
	rm -f $(BINDIR)/vitunes

### test build (using gtest)

CXX 			?= clang++
TEST_CFLAGS	 = $(CDEPS) -c -g -Wall -Wextra -I/usr/local/include
TEST_LIBS	 = $(LDEPS) -L/usr/local/lib -lgtest -lgtest_main -lpthread -lsqlite3
TEST_OBJS=mfile.t.o \
			 mfile_taglib.t.o \
			 plist.t.o \
			 indent.t.o \
			 mediadb.t.o \
			 dparray.t.o

# XXX this needs work - either...
# 	add $(OBJS) to the link and have the test include .h's instead
# -or-
#  something else
test: $(TEST_OBJS)
	$(CXX) $(TEST_LIBS) -o $@ $(TEST_OBJS)
	./test --gtest_color=yes

.cc.o:
	$(CXX) $(TEST_CFLAGS) $<

