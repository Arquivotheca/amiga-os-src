
STRTUP   =  LIB:astartup.obj
OBJS     =  amouse.o jmouse.obj mouseimage.o
LIBS     =  LIB:janus.lib LIB:amiga.lib LIB:lc.lib
       
CC       =  lc
CCF      =  -v
AS       =  assem
ASF      =  -c w200000 -i ram:include -o
LN       =  blink

amouse : $(OBJS)
   $(LN) from $(STRTUP) $(OBJS) to amouse lib $(LIBS)

jmouse.obj :   jmouse.asm
   $(AS) jmouse.asm $(ASF) jmouse.obj

amouse.o :   amouse.c
   $(CC) $(CCF) amouse

mouseimage.o : mouseimage.c
   $(CC) $(CCF) mouseimage
