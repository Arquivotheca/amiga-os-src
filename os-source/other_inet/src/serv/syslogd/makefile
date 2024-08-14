#
# Makefile for syslogd
#

NETLIB	= NETBIN:lib/socket32.lib
CLIB	= c32.lib
BIN	= NETBIN:serv
OBJ	= syslogd.o

.c.o:
	cc  -DAMIGA +l -Inetinc: $*.c

all:	syslogd

syslogd:	version.o 
	ln +ss -m +q -o $@ version.o $(OBJ) $(NETLIB) $(CLIB)

version.o: $(OBJ)
	vers >version.c -v syslogd
	cc version.c

clean:
	delete #?.o

install:	syslogd
	copy syslogd $(BIN) clone
