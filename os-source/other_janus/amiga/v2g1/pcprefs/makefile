STRTUP   =  LIB:astartup.obj
OBJS     =  setprefs.obj mainprog.obj gadgets.obj
LIBS     =  LIB:janus.lib LIB:amiga.lib
       
CC       =  lc
CCS      =  -v -dDEBUG=1
CCF      =  -v
AS       =  assem
ASF      =  -c w200000 -i ram:include -o
LN       =  blink

pcprefs : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to pcprefs lib $(LIBS)

setprefs.obj :   setprefs.asm mydata.i
   $(AS) setprefs.asm $(ASF) setprefs.obj

mainprog.obj :   mainprog.asm mydata.i
   $(AS) mainprog.asm $(ASF) mainprog.obj

gadgets.obj :   gadgets.asm
   $(AS) gadgets.asm $(ASF) gadgets.obj

