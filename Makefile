#ident "$Id$"
## -----------------------------------------------------------------------
##   
##   Copyright 2005 H. Peter Anvin - All Rights Reserved
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation, Inc., 53 Temple Place Ste 330,
##   Boston MA 02111-1307, USA; either version 2 of the License, or
##   (at your option) any later version; incorporated herein by reference.
##
## -----------------------------------------------------------------------


#
# Makefile for memhack
#

CC       = gcc -Wall
CFLAGS   = -g -O2 -fomit-frame-pointer -D_GNU_SOURCE -D_LARGE_FILE -D_FILE_OFFSET_BITS=64
LDFLAGS  = 

BIN	= getmem setmem getio setio

sbindir = /usr/sbin

all: $(BIN)

clean:
	rm -f *.o $(BIN)

distclean: clean
	rm -f *~ \#*

install: all
	install -m 755 $(BIN) $(sbindir)

.o:
	$(CC) $(LDFLAGS) -o $@ $<

.c.o:
	$(CC) $(CFLAGS) -o $@ $<

.c:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<
