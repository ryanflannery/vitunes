include ../config.mk

# install locations (overridden in config.mk above)
PREFIX?=/usr/local
MANDIR?=$(PREFIX)/man/man1

MAN_PAGES=vitunes.1 \
	  		 vitunes-add.1 \
	  		 vitunes-addurl.1 \
	  		 vitunes-check.1 \
	  		 vitunes-flush.1 \
	  		 vitunes-init.1 \
	  		 vitunes-rm.1 \
	  		 vitunes-tag.1 \
	  		 vitunes-update.1

.PHONY: lint install uninstall

.DEFAULT:
	@echo Nothing to do here

install:
	mkdir -p $(MANDIR)
	install -c -m 0444 $(MAN_PAGES) $(MANDIR)

uninstall:
	cd $(MANDIR)
	rm -f $(MAN_PAGES)

lint:
	mandoc -Tlint $(MAN_PAGES)
