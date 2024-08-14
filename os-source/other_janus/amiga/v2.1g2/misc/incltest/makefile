STRTUP   =  LIB:c.o
OBJS     =  testc.o
LIBS     =  LIB:janus.lib+LIB:lc.lib+LIB:amiga.lib
       
CC       =  lc
CCS      =  -v -dDEBUG=1
CCF      =
AS       =  assem
ASF      =  -c w200000 -i ram:include -o
LN       =  blink

testc.o :   testc.c
   $(CC) $(CCF) testc.c

testa.obj : testa.asm
   $(AS) testa.asm $(ASF) testa.obj
