# NATIVE BUILD ONLY

SU    = lib:c.o
CC    =lc
xCCF   =-csf -v -b0 -O -rr -d -q500w1e -iv36:include -iv36:inc.lattice
CCF   =-csf -v -b0 -O -d -q500w1e
xAS  = assem
AS  = asm
xASF = -c w200000 -i fh1:languages/latticec5.02/lc/include -o
xASF = -c w200000 -i v36:include -o
ASF = -iv36:include/
LN    =blink
LNF   =NODEBUG SMALLCODE SMALLDATA BATCH
LIBS  =lib:lc.lib+lib:amiga.lib+lib:debug.lib

SRC=startloc.c

OBJ=startloc.o

.asm.o:
   $(AS) $(ASF) $>.asm

.c.o:
   $(CC) $(CCF) $>

startloc.ld: $(OBJ) makefile
   $(LN) $(LNF) FROM $(SU) $(OBJ) LIB $(LIBS) TO $@
