# NATIVE BUILD ONLY

SU=lib:c.o
CC=lc
xCCF=-csf -v -o
CCF=-csf -v -d -b0 -q500w1e -o
LN=blink
LNF=NODEBUG SMALLCODE SMALLDATA DEFINE __main=__tinymain
LIBS=lib:lc.lib+lib:janus.lib+lib:amiga.lib+lib:debug.lib

INC=amouse.h
OBJ=telluser.o amouse.o

amouse.ld: $(OBJ)
   $(LN) $(LNF) FROM $(SU) $(OBJ) LIB $(LIBS) TO $@

amouse.o : amouse.c $(INC)
   $(CC) $(CCF)$@ amouse.c

telluser.o : telluser.c $(INC)
   $(CC) $(CCF)$@ telluser.c

.asm.obj:
   assem $*.asm -c w200000 -i fh1:languages/latticec5.02/lc/include -o $*.obj
