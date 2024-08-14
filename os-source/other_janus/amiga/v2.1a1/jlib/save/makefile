STRTUP     = LIB:astartup.obj
xOBJS      = abs.obj alloc.obj alloc2.obj autoload.obj 
OBJS       = abs.obj alloc.obj alloc2.obj \
             ints.obj init.obj lib.obj defs.obj amigarw.obj \
             runback.obj services.obj servicesup.obj \
             sfuncs1.obj sfuncs1b.obj sfuncs2.obj sfuncs3.obj sfuncs4.obj \
             gets.o endcode.obj
SRC        = abs.asm alloc.asm alloc2.asm \
             ints.asm init.asm lib.asm defs.asm amigarw.asm runback.asm \
             services.asm servicesup.asm \
             sfuncs1.asm sfuncs1b.asm sfuncs2.asm sfuncs3.asm sfuncs4.asm \
             gets.c endcode.asm
LIBS       = LIB:janus.lib+LIB:debug.lib+LIB:amiga.lib
xLIBS      = LIB:janus.lib+expansio.lib+LIB:debug.lib+LIB:amiga.lib
EXTDISK    = df2
CC         = lc
CCF        = -v 
AS         = assem
ASF        = -c w200000 -i fh1:languages/latticec5.02/lc/include -o
xLN         = alink
LN        = blink

.asm.obj:
   $(AS) $*.asm $(ASF) $*.obj

.c.o:
   $(CC) $(CCF) $*.c

janus.library: $(OBJS)
   $(LN) with lib.wth

janus.lib:   linkage.asm lvos.asm
   $(AS) linkage.asm $(ASF) linkage.obj
   $(AS) lvos.asm $(ASF) lvos.obj
   join lvos.obj linkage.obj as janus.lib

adoc: $(SRC)
   type  >ram:temp.adoc abs.asm
   type >>ram:temp.adoc alloc.asm
   type >>ram:temp.adoc alloc2.asm
   type >>ram:temp.adoc amigarw.asm
   type >>ram:temp.adoc defs.asm
   type >>ram:temp.adoc endcode.asm
   type >>ram:temp.adoc init.asm
   type >>ram:temp.adoc ints.asm
   type >>ram:temp.adoc lib.asm
   type >>ram:temp.adoc linkage.asm
   type >>ram:temp.adoc lvos.asm
   type >>ram:temp.adoc process.asm
   type >>ram:temp.adoc putmsg.asm
   type >>ram:temp.adoc runback.asm
   type >>ram:temp.adoc services.asm
   type >>ram:temp.adoc servicesup.asm
   type >>ram:temp.adoc sfuncs1.asm
   type >>ram:temp.adoc sfuncs1b.asm
   type >>ram:temp.adoc sfuncs2.asm
   type >>ram:temp.adoc sfuncs3.asm
   type >>ram:temp.adoc sfuncs4.asm
   :source/autodoc/autodoc >janus.library.adoc -a -I ram:temp.adoc

o:    $(OBJS)

