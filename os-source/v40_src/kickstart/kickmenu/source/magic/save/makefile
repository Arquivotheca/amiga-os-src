# NATIVE BUILD ONLY

SU    =
CC    =lc
CCF   =-csf -v -b0 -O -rr -d -q500w1e -iv36:include -iv36:inc.lattice
xAS  = assem
AS  = asm
xASF = -c w200000 -i fh1:languages/latticec5.02/lc/include -o
xASF = -c w200000 -i v36:include -o
ASF = -iv36:include/
LN    =blink
LNF   =NODEBUG SMALLCODE SMALLDATA BATCH
LIBS  =lib:debug.lib+lib:amiga.lib+lib:lcr.lib+endcode.lib

SRC=rtag.asm init.c

OBJ=rtag.o init.o

.asm.o:
   $(AS) $(ASF) $>.asm

.c.o:
   $(CC) $(CCF) $>

Magic.ld: $(OBJ) makefile endcode.lib
   v36:aug/bin/bumprev 36 Magic_rev
   delete Magic_rev.h
   $(AS) $(ASF) rtag.asm
   $(LN) $(LNF) FROM $(SU) $(OBJ) LIB $(LIBS) TO $@

endcode.lib: endcode.asm
   assem -o endcode.lib endcode.asm
