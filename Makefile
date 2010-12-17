# install locations
BINDIR=/usr/local/bin
MANDIR=/usr/local/man/man1

# non-base dependency build flags
CDEPS=`taglib-config --cflags`
LDEPS=`taglib-config --libs` -ltag_c

CC?=/usr/bin/gcc
CFLAGS+=-c -std=c89 -Wall -Wextra -Wno-unused-value -g $(CDEPS)
LDFLAGS+=-lm -lncurses -lutil $(LDEPS)

OBJS=input_handlers.o e_commands.o medialib.o meta_info.o \
	  paint.o player.o playlist.o str2argv.o uinterface.o \
	  vitunes.o

.PHONY: clean install uninstall

vitunes: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o
	rm -f vitunes vitunes.core
	rm -f test_str2argv


install: vitunes
	/usr/bin/install -c -m 0555 vitunes $(BINDIR)
	/usr/bin/install -c -m 0444 vitunes.1 $(MANDIR)

uninstall:
	rm -f $(BINDIR)/vitunes
	rm -f $(MANDIR)/vitunes.1

cscope.out: *.h *.c
	cscope -bke

# test program for str2argv
test_str2argv:	str2argv.h str2argv.c
	$(CC) $(CFLAGS) -Dstr2argv_test_main=main -o test_str2argv.o str2argv.c
	$(CC) $(LDFLAGS) -o $@ test_str2argv.o

