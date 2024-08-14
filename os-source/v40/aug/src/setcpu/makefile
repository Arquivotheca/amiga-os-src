######################################################################
#
# Makefile for SetCPU V1.5
#
######################################################################

.a.o:
	as -o $@ $*.a

CFLAGS	= -n +x5
LFLAGS	= -g -lm -lc

COBJS	= strings.o expdev.o devio.o memory.o patches.o coolhand.o setcpu.o
OBJS	= 030stuff.o $(COBJS)

SetCPU: 	$(OBJS) setcpu.o
		ln +Q $(OBJS) -o SetCPU $(LFLAGS)

$(COBJS):       setcpu.h

tst:		030Stuff.o tst.o
		ln +Q 030Stuff.o tst -o tst $(LFLAGS)

KickFile:	strings.o devio_kf.o memory_kf.o kickfile.o
		ln +Q strings.o devio_kf.o memory_kf.o kickfile.o -o KickFile $(LFLAGS)

memory_kf.o:
		cc $(CFLAGS) memory.c -dKICKFILE -o memory_kf.o
devio_kf.o:
		cc $(CFLAGS) devio.c -dKICKFILE -o devio_kf.o

internal:
		copy makefile ghostwheel:amiga/V36/aug/src/setcpu
		copy setcpu ghostwheel:amiga/V36/aug/bin
		copy kickfile ghostwheel:amiga/V36/aug/bin
		copy /setcpu.txt ghostwheel:amiga/V36/aug/doc/setcpu.doc
		copy "#?.c" ghostwheel:amiga/V36/aug/src/setcpu
		copy "#?.h" ghostwheel:amiga/V36/aug/src/setcpu
		copy "#?.a" ghostwheel:amiga/V36/aug/src/setcpu
