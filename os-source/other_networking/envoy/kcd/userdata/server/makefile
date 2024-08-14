all:	server

%.o: %.c
	lc -O -rr -v -o$@ $<

server : server.o
	blink server.o to server lib lib:amiga.lib lib:debug.lib SC SD

server.o : server.c server_internal.h /transactions.h

protos:
	lc -pr -DPROTOS=1 -v -oserver_protos.h server.c
