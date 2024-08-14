STRTUP   =  LIB:c.o
OBJS     =  cga.o ref80.o renderchar.o dcur.o int.o
LIBS     =  LIB:janus.lib+LIB:lc.lib+LIB:amiga.lib
       
CC       =  lc
CCS      =  -v -dDEBUG=1
CCF      =  -v -d -dDEBUG
AS       =  assem
ASF      =  -c w200000 -i ram:include -o
LN       =  blink

CGA : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to CGA lib $(LIBS)

cga.o :   cga.c
   $(CC) $(CCF) cga.c

ref80.o :   ref80.a
   asm ref80.a

renderchar.o :    renderchar.a
   asm renderchar.a

dcur.o :    dcur.a
   asm dcur.a
int.o :    int.a
   asm int.a


