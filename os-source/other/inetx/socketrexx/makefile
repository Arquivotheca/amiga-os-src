#
# Makefile for socketrexx
#

NETLIB	= netlib:socket32.lib
CLIB	= c32.lib
MLIB	= m32.lib
BIN	= inet:c
OBJ	= rexxcmd.o rexxnet.o rexxports.o sasin.o rexxerr.o rexxio.o \
	  rexxrvi.o rexxsock.o rexxif.o rexxlookup.o rexxsocket.o rexxglue.o \
	  rexxvars.o
.c.o:
	cc -n -DAMIGA +l -I. -Inetinc: $*.c

all:	socketrexx

rexxvars.o: rexxvars.obj
	copy rexxvars.obj rexxvars.o

socketrexx: 	version.o
	ln -g -m +q -o $@ version.o $(OBJ) $(NETLIB) $(CLIB)

version.o: 	$(OBJ)
	vers >version.c -v socketrexx
	cc version.c

clean:
	delete #?.o #?.dbg

install:	socketrexx
	copy socketrexx $(BIN)
