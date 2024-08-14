STRTUP   =  LIB:astartup.obj
OBJS     =  start.obj maintask.obj
LIBS     =  LIB:janus.lib cprintf.lib LIB:amiga.lib
       
CC       =  lc
CCF      =  -v -d
AS       =  assem
ASF      =  -c w200000 -i vd0:include -o
LN       =  blink

pcdisk.ld : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to pcdisk.ld lib $(LIBS)

start.obj :   start.asm mydata.i
   $(AS) start.asm $(ASF) start.obj

maintask.obj :   maintask.asm mydata.i
   $(AS) maintask.asm $(ASF) maintask.obj
