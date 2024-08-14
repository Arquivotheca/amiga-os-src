STRTUP   =
OBJS     = abs.obj,alloc.obj,ints.obj,init.obj,lib.obj,defs.obj,amigarw.obj,endcode.obj
OBJ1     = abs.obj alloc.obj ints.obj init.obj lib.obj defs.obj amigarw.obj endcode.obj
LIBS     =  janus.lib,debug.lib,LIB:amiga.lib
       
CC       =  lc
CCF      =  -v -d
AS       =  assem
ASF      =  -c w200000 -i ram:include -o
LN       =  alink

janus.library : $(OBJ1)
   $(LN) from $(STRTUP) $(OBJS) to PCBOOT:janus.library library $(LIBS)

janus.lib : linkage.asm lvos.asm
   $(AS) linkage.asm $(ASF) linkage.obj
   $(AS) lvos.asm    $(ASF) lvos.obj
   join lvos.obj linkage.obj as janus.lib

abs.obj :   abs.asm
   $(AS) abs.asm $(ASF) abs.obj

alloc.obj :   alloc.asm
   $(AS) alloc.asm $(ASF) alloc.obj

ints.obj :   ints.asm
   $(AS) ints.asm $(ASF) ints.obj

init.obj :   init.asm
   $(AS) init.asm $(ASF) init.obj

lib.obj :   lib.asm
   $(AS) lib.asm $(ASF) lib.obj

defs.obj :   defs.asm
   $(AS) defs.asm $(ASF) defs.obj

amigarw.obj :   amigarw.asm
   $(AS) amigarw.asm $(ASF) amigarw.obj

endcode.obj :   endcode.asm
   $(AS) endcode.asm $(ASF) endcode.obj

