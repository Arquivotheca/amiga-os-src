#  Manx 5.0d makefile.  You should modify your Manx environment variables
#  to search the socket library headers and link libraries.  Your
#  manx-statrup script might look something like this:
#
#		mset  INCLUDE=manx:include!work:socklib/include!MANX:asm!
#		mset  CLIB=MANX:libs!work:socklib/lib!
#

FLAGS = -wlp

ncopy.o transfer.o server.o: ncopy.h
ncopy.o transfer.o: client.h


ncopy: ncopy.o transfer.o
	ln ncopy.o transfer.o -lc +l amiga.lib debug.lib

server1: server1.o
	ln server1.o -lc +l amiga.lib

server2: server2.o
	ln server2.o -lc +l amiga.lib

server3: server3.o
	ln server3.o -lc +l amiga.lib

all: ncopy server1 server2 server3
