# kodiak's wack makefile
# $Id: makefile,v 1.14 92/05/26 14:13:37 peter Exp $

#TARGET   = /usr/commodore/bin
#TARGET20   = /usr/commodore/bin20
#SOURCED= /batgirl/pub/src/wack

# $V/utilities version
TARGET   = ../../bin
SOURCED= .

VERSION = 39

INSTALLCP = cp

CURRENT= main.o actions.o symload.o decode.o nmprintf.o \
	memory.o symbols.o show.o special.o ops.o initial.o sat.o \
	gather.o define.o globals.o ishexnum.o interp.o  hunks.o \
	showd.o showg.o showi.o showibase.o showlock.o find.o copper.o

CFLAGS= -DLOCALI
#CFLAGS=

.SUFFIXES: .asm .ol .c

INCDIRS=-I/usr/include \
	-I/usr/commodore/amiga/V$(VERSION)/src/kickstart \
	-I/usr/commodore/amiga/V$(VERSION)/include

.c.o:
	cc -g -c $(CFLAGS) $(INCDIRS) $< 

# we don't have these tools anymore: don't trash the .ol files!!
#.asm.o:
#	masm $< -o $*.ol
#	rcon $*.ol -n

# dig this:
#.asm.o:
#	@echo cannot build $*.o
#	rm -f $*.o
#	cp $(SOURCED)/Precious/$*.o $*.o
#	chmod 660 $*.o

# Dig this instead: We do _so_ have these tools!
.asm.o:
	masm2.0 $< -o $*.ol
	rcon2.0 $*.ol -n

a:	$(CURRENT)
	cc -g -o a $(CURRENT)

install: a
	$(INSTALLCP) a $(TARGET)/wack
#	$(INSTALLCP) a $(TARGET20)/wack

Log:
	changelog  -d ./RCS -u -f Log *.c *.h *.asm *.i

tags:
	ctags *.c *.h

#showi.o:	intuinternal.h
find.o:		find.c
showd.o:	showd.c
showi.o:	showi.c
showibase.o:	showibase.c
showg.o:	showg.c
showlock.o:	showlock.c
sat.o:		sat.c
main.o:		main.c
define.o:	define.c
actions.o:	actions.asm
symload.o:	symload.c
decode.o:	decode.c
memory.o:	memory.c
symbols.o:	symbols.c
nmprintf.o:	nmprintf.asm
show.o:		show.c
special.o:	special.c
initial.o:	initial.c
gather.o:	gather.c
define.o:	define.c
globals.o:	globals.c
ishexnum.o:	ishexnum.asm
interp.o:	interp.c
hunks.o:	hunks.c
