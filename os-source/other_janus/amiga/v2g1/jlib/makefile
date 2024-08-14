STRTUP     = LIB:astartup.obj
xOBJS      = abs.obj alloc.obj alloc2.obj autoload.obj 
OBJS       = abs.obj alloc.obj alloc2.obj \
             ints.obj init.obj lib.obj defs.obj amigarw.obj runback.obj \
             services.obj servicesup.obj \
             sfuncs1.obj sfuncs1b.obj sfuncs2.obj sfuncs3.obj sfuncs4.obj \
             endcode.obj
LIBS       = LIB:janus.lib+LIB:debug.lib+LIB:amiga.lib
xLIBS      = LIB:janus.lib+expansio.lib+LIB:debug.lib+LIB:amiga.lib
EXTDISK    = df2
CC         =  
CCF        =  
AS         = assem
ASF        = -c w200000 -i ram:include -o
LN         = alink
xLN        = blink

.asm.obj:
   $(AS) $*.asm $(ASF) $*.obj

.c.o:
   lc1 -iINCLUDE: $*.c
   lc2 -v $*

janus.library: $(OBJS)
   $(LN) with lib.wth

janus.lib:   linkage.asm lvos.asm
   $(AS) linkage.asm $(ASF) linkage.obj
   $(AS) lvos.asm $(ASF) lvos.obj
   join lvos.obj linkage.obj as janus.lib

o:    $(OBJS)

xcopy: 
   copy janus.library zaphod:expansion

copy: 
   copy janus.library $(EXTDISK):expansion

all:
   make

xall:
   del #?.obj
   del #?.o
   make all

