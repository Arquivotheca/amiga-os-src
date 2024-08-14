STRTUP   =  lib:c.o
OBJS     =  djmount.o
LIBS     =  LIB:janus.lib+LIB:lc.lib+LIB:amiga.lib
       
CC       =  lc
CCS      =  -v -d -dDEBUG=1
CCF      =  -v -d
AS       =  assem
ASF      =  -c w200000 -i ram:include -o
LN       =  blink

djmount : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to djmount lib $(LIBS)

djmount.o :   djmount.c
   $(CC) $(CCF) djmount.c

