LOCAL = h:srca/io_torture
REMOTE = V36:aug.cas/src/io_torture

MYLIBS= LIB:debug.lib

LC = lc
ASM = HX68

# Note - io_main.c copied to io_main_par.c by Makefile
CFILES=		io_main.c io_main_par.c
AFILES=		io_torture.asm
OFILES=		io_main.o io_torture.o
SFILES=		io_main.o io_torture.o
DFILES=		io_main_par.o io_torture.o

LFLAGS= SMALLCODE BATCH ND
LCFLAGS  = -b0 -cfistq -v -y -j73
LCFLAGSP = -b0 -cfistq -v -y -j73 -dPARALLEL
#AFLAGS = -iV36:include

.SUFFIXES:
.SUFFIXES:	.o .c .h .asm .i

io_torture: $(OFILES)
	echo "making serial version"
	copy io_main.c io_main_par.c
	blink FROM Astartup.obj $(SFILES) LIB LIB:amiga.lib  \
	LIB:lcnb.lib LIB:debug.lib TO io_torture ${LFLAGS}

	echo "making parallel version"
	$(LC) $(LCFLAGSP) io_main_par.c
	blink FROM Astartup.obj $(DFILES) LIB LIB:amiga.lib  \
	LIB:lcnb.lib LIB:ddebug.lib TO io_torture.par ${LFLAGS}


.c.o:
	$(LC) $(LCFLAGS) $*.c
.asm.o:
	HX68 -A $*.asm -iINCLUDE: -o$*.o -cRQ

io_main.o:	io_main.c
io_main_par.o:	io_main_par.c
io_torture.o:	io_torture.asm
io_main.c:	io_torture_rev.h
io_torture.asm: io_torture_rev.i

version:
	@bumprev 36 io_torture_rev

local:
	@copy \#?.c|\#?.asm|\#?.h|\#?.i|Makefile|\#?|lmkfile|\#?.rev $(LOCAL)

internal:
	@copy \#?.c|\#?.asm|\#?.h|\#?.i|Makefile|\#?|lmkfile|\#?.rev $(REMOTE)

clean:
	@delete \#?.o|\#?


