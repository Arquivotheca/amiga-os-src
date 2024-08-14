# kodiak's wack makefile
# $Id: makefile,v 1.15 92/08/27 12:51:15 peter Exp $


# $V/utilities version
TARGET   = ../../bin
SOURCED= .

VERNUM = 39
VERDIR=		/usr/commodore/amiga/V${VERNUM}
SRCPATH= ${VERDIR}/utilities/wack

INSTALLCP = cp
MMINSTALL=	${VERDIR}/tools/sh/mminstall

CURRENT= main.o actions.o symload.o decode.o nmprintf.o \
	memory.o symbols.o show.o special.o ops.o initial.o sat.o \
	gather.o define.o globals.o ishexnum.o interp.o  hunks.o \
	showd.o showg.o showi.o showibase.o showlock.o find.o copper.o

CFILES= copper.c cprmain.c decode.c define.c dmpcpr.c find.c \
	frame.c gather.c globals.c hunks.c initial.c interp.c \
	main.c memory.c ops.c sat.c show.c showd.c showg.c \
	showhandler.c showi.c showibase.c showlock.c special.c \
	symbols.c symload.c

HFILES= dosmagic.h header.h hunks.h semaphore.h special.h \
	std.h symbols.h wack.h watch.h

AFILES= actions.asm nmprintf.asm ishexnum.asm

IFILES= tokens.i

MISCFILES= HOW_TO README checkin makefile wack.notes2 wack


CFLAGS= -DLOCALI
#CFLAGS=

.SUFFIXES: .asm .ol .c

INCDIRS=-I/usr/include \
	-I/usr/commodore/amiga/V$(VERNUM)/src/kickstart \
	-I/usr/commodore/amiga/V$(VERNUM)/include

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

internal:
	cp a wack
	${MMINSTALL} ${VERDIR}/utilities/bin wack
	${MMINSTALL} /usr/commodore/bin wack

	${MMINSTALL} ${SRCPATH} ${CFILES} ${AFILES} ${HFILES} ${IFILES} ${MISCFILES}

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
