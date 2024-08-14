#
# Makefile for tftpd
#

NETLIB	= net32.lib
CLIB	= c32.lib
BIN	= inet:serv
OBJ	= tftpd.o tftpsubs.o

.c.o:
	cc -DAMIGA +l -I//include $*.c

all:	tftpd

tftpd:	version.o a:lib/net/net32.lib
	ln +ss -m +q -o $@ version.o $(OBJ) $(NETLIB) $(CLIB)

version.o: $(OBJ)
	vers >version.c -v tftpd
	cc version.c

clean:
	delete #?.o

install:	tftpd
	copy tftpd $(BIN) clone
