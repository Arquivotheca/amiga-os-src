#
# Makefile for nterm
#

NETLIB	= net32.lib
CLIB	= c32.lib
BIN	= inet:c

#
# Normally we don't use -S, but this program just isn't written in
# correct C.
#
.c.o:	
	cc  -DMANX -S -E2000  +l -I//include $*.c

OBJ =	nterm.o init.o console.o functions.o keymap.o vt100.o \
	tabs.o request.o quit.o main.o ctrl.o tty.o who.o  meta.o \
	overreq.o amiga_ctrl.o menu.o rmenu.o rlogin.o fonts.o colors.o \


all:	install
	
rloginVT:	version.o a:lib/net/net32.lib
	ln -m +q -o $@ version.o $(OBJ) $(NETLIB) $(CLIB)

version.o:	$(OBJ)
	vers >version.c -v nterm
	cc version.c

install:	rloginVT
	copy rloginVT $(BIN) clone

clean:
	-delete #?.o
