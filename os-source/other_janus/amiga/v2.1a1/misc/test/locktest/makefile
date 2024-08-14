STRTUP   =  LIB:c.o
OBJS     =  lockserv.o lock.obj
LIBS     =  LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib+lib:debug.lib
       
CC       =  lc
CCS      =  -v -dDEBUG=1
CCF      =  -v -d
AS       =  assem
ASF      =  -c w200000 -i std:latticec/include -o
LN       =  blink

lockserv : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to lockserv lib $(LIBS)

lockserv.o :   lockserv.c lockserv.h
   $(CC) $(CCF) lockserv.c

lock.obj : lock.asm
   $(AS) lock.asm $(ASF)  lock.obj

