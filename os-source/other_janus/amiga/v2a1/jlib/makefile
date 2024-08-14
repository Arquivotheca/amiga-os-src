STRTUP    =  LIB:astartup.obj
OBJS      =  abs.obj alloc.obj alloc2.obj \
             ints.obj init.obj lib.obj defs.obj amigarw.obj \
             services.obj \
             servicesup.obj sfuncs1.obj sfuncs2.obj sfuncs3.obj \
             endcode.obj
TESTOBJS  =  test.o 
SPEEKOBJS =  speek.o 
PARTYOBJS =  partyline.o
LIBS      =  LIB:janus.lib+LIB:debug.lib+LIB:amiga.lib
xLIBS     =  LIB:janus.lib+expansio.lib+LIB:debug.lib+LIB:amiga.lib
EXTDISK   =  df0
CC        =  
CCF       =  
AS        =  assem
ASF       =  -c w200000 -i ram:include -o
LN        =  alink
xLN       =  blink

.asm.obj:
   $(AS) $*.asm $(ASF) $*.obj

.c.o:
   lc1 -iINCLUDE: $*.c
   lc2 -v $*

janus.library: $(OBJS)
   $(LN) with lib.wth

janus.lib:   linkage.obj lvos.obj
   $(AS) linkage.asm $(ASF) linkage.obj
   $(AS) lvos.asm $(ASF) lvos.obj
   join lvos.obj linkage.obj as janus.lib

test:   $(TESTOBJS)
   blink FROM LIB:astartup.obj $(TESTOBJS) \
      to test LIB LIB:amiga.lib \
      LIB:lc.lib janus.lib LIB:debug.lib

speek:   $(SPEEKOBJS)
   blink FROM LIB:astartup.obj $(SPEEKOBJS) \
      to speek LIB LIB:amiga.lib \
      LIB:lc.lib janus.lib LIB:debug.lib


all:
   make
   make janus.lib
   make test
   make speek

xall:
   del #?.obj
   del #?.o
   make all

