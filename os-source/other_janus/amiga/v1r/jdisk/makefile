STRTUP   =  LIB:astartup.obj
OBJS     =  djmount.obj
LIBS     =  LIB:janus.lib+cprintf.lib+LIB:amiga.lib
       
CC       =  lc
CCS      =  -v -dDEBUG=1
CCF      =  -v
AS       =  assem
ASF      =  -c w200000 -i ram:include -o
LN       =  blink

djmount : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to djmount lib $(LIBS)

djmount.obj :   djmount.asm
   $(AS) djmount.asm $(ASF) djmount.obj

