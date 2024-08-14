#
# MakeFile for MKS_Lens...
#

#
# ASSEM 10.178
#.asm.o:
#	Assem -i Lattice:Include -o $*.o $*.asm -c w200000

#
# CAPE  68k
.asm.o:
	CAsm -a $*.asm -i Include: -cq -o $*.o

#
# AS 68K from MANX
#.asm.o:
#	as -D -C -o $*.o $*.asm

#
# ASM from Lattice
#.asm.o:
#	asm -iInclude: $*.asm

#
# ArgAsm
#.asm.o:
#	Arg $*.asm -i Include: -o $*.o


CFLAGS= -b1 -cfistq -d1 -ms0 -v -rr

.c.o:
	@LC $(CFLAGS) $*

#
#

OBJS=	Magnify.o MKS_Lens.o
LIBS=	LIB:lcr.lib

MKS_Lens: MKS_Lens.ld
	BLINK FROM MKS_Lens.ld TO MKS_Lens NODEBUG

MKS_Lens.ld: $(OBJS) $(LIBS)
	BLink FROM LIB:c.o $(OBJS) TO MKS_Lens.ld LIB $(LIBS) DEFINE @_main=@_tinymain SMALLDATA SMALLCODE

Magnify.o: Magnify.asm
