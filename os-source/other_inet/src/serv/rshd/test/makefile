#
# Makefile for inetd
#

NETINC = NETINC:
OBJ	= main.o inettime.o readn.o udpopen.o tcpopen.o readline.o dgsendrecv.o rresvport.o

.c.o:
	lc -v -cfist -O -i$(NETINC) -o$@ $*.c

all:	inettime

inettime:	$(OBJ)
	blink FROM lib:c.o $(OBJ) LIB lib:lc.lib TO $@ MAP SC SD ND 

clean:
	delete #?.o

