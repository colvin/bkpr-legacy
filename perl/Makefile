INSTALL	= install -C

PREFIX	= /usr/local
DESTDIR	?= $(PREFIX)

BINDIR	= $(DESTDIR)/bin
LIBDIR	= $(DESTDIR)/lib/perl5/site_perl
MANDIR	= $(DESTDIR)/man/man1

BINUSR	= root
BINGRP	= wheel
BINMOD	= 775
BINARY	= bkpr

LIBUSR	= $(BINUSR)
LIBGRP	= $(BINGRP)
LIBMOD_F	= 664
LIBMOD_D	= 775

MODS = \
	info\
	init\
	list\
	create\
	destroy\
	start\
	update\
	status\
	kill\
	test


all: modules binary man

binary:
	$(INSTALL) -o $(BINUSR) -g $(BINGRP) -m $(BINMOD) bkpr.pl $(BINDIR)/$(BINARY)

modules:
	$(INSTALL) -o $(LIBUSR) -g $(LIBGRP) -m $(LIBMOD_F) BKPR.pm $(LIBDIR)/BKPR.pm
	mkdir -p $(LIBDIR)/BKPR/
.for m in $(MODS)
	$(INSTALL) -o $(LIBUSR) -g $(LIBGRP) -m $(LIBMOD_F) BKPR/$(m).pm $(LIBDIR)/BKPR/
.endfor

man:
	gzip -c bkpr.1 > $(MANDIR)/bkpr.1.gz

help:
	@echo "DESTDIR: $(DESTDIR)"
	@echo "binary:  $(BINDIR)/$(BINARY)"
	@echo "modules: $(LIBDIR)/BKPR/"

