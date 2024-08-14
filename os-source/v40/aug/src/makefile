#
#   Makefile for Lattice Amiga C 5.02
#
.asm.o:
	c $*

.c.o:
	lc -d2 -v -iINCLUDE: -cs $*


OBJFILES1   = LIB:c.o TinyMain.o LoadF.o LoadSeg_Old.o TinyPrintf.o
RESULT1     = LoadF
loadf: $(OBJFILES1)
	blink CHIP FROM $(OBJFILES1) TO $(RESULT1) LIB LIB:lc.lib LIB:amiga.lib MAP $(RESULT1).map,l SMALLDATA SMALLCODE NODEBUG


OBJFILES2   = ClearF.o
RESULT2     = ClearF
clearf: $(OBJFILES2)
	blink CHIP FROM $(OBJFILES2) TO $(RESULT2) LIB LIB:amiga.lib NODEBUG


OBJFILES3   = LIB:c.o TinyMain.o KillF.o TinyPrintf.o
RESULT3     = KillF
killf: $(OBJFILES3)
	blink CHIP FROM $(OBJFILES3) TO $(RESULT3) LIB LIB:lc.lib LIB:amiga.lib MAP $(RESULT1).map,l SMALLDATA SMALLCODE NODEBUG

internal:
	copy makefile ghostwheel:amiga/V36/aug/src/f_utils
	copy loadf ghostwheel:amiga/V36/aug/bin
	copy killf ghostwheel:amiga/V36/aug/bin
	copy clearf ghostwheel:amiga/V36/aug/bin
	copy loadf a:mytools
	copy killf a:mytools
	copy clearf a:mytools
	copy loadf a:first_aid
	copy killf a:first_aid
	copy clearf a:first_aid
	copy f_utils.doc ghostwheel:amiga/V36/aug/doc
	copy "\#?.c" ghostwheel:amiga/V36/aug/src/f_utils
	copy "\#?.asm" ghostwheel:amiga/V36/aug/src/f_utils
