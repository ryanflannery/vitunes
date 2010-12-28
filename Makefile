# install locations
PREFIX?=/usr/local
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/man/man1

# non-base dependency build flags
CDEPS=`taglib-config --cflags`
LDEPS=`taglib-config --libs` -ltag_c

CC?=/usr/bin/gcc
CFLAGS+=-c -std=c89 -Wall -Wextra -Wno-unused-value -g $(CDEPS)
LDFLAGS+=-lm -lncurses -lutil $(LDEPS)

.ifdef debug
CFLAGS+=-DDEBUG -g
.endif

OBJS=input_handlers.o e_commands.o medialib.o meta_info.o \
	  paint.o player.o playlist.o str2argv.o uinterface.o \
	  vitunes.o

.PHONY: debug clean install uninstall publish-repos

vitunes: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $<

debug:
	make debug=1

clean:
	rm -f *.o
	rm -f vitunes vitunes.core vitunes-debug.log
	rm -f test_str2argv


install: vitunes
	/usr/bin/install -c -m 0555 vitunes $(BINDIR)
	/usr/bin/install -c -m 0444 vitunes.1 $(MANDIR)

uninstall:
	rm -f $(BINDIR)/vitunes
	rm -f $(MANDIR)/vitunes.1

cscope.out: *.h *.c
	cscope -bke

publish-repos:
	hg push $(myhg)/vitunes
	hg push $(mybb)/vitunes
	hg push $(mygit)/vitunes

# test program for str2argv
test_str2argv:	str2argv.h str2argv.c
	$(CC) $(CFLAGS) -Dstr2argv_test_main=main -o test_str2argv.o str2argv.c
	$(CC) $(LDFLAGS) -o $@ test_str2argv.o

