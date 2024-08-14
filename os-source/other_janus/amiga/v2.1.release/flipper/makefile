SU   = V:lib/lattice/c.o
OBJ  = main.o display.o disk.o janus.o change.o stringutils.o debug.o inhibit.o startup.o 

DEFINES = -DWB_SUPPORT=1 
       
CC   = lc
CCF  = -v -cat -b0 -HV:include/all.ph -iV:include -iV:inc.lattice

LN   = blink
LNF  = SC SD DEFINE __main=__tinymain
LIBS = V:lib/lattice/lc.lib+V:lib/amiga.lib+V:lib/janus.lib+/support/support.lib

.c.o :
   $(CC) $(CCF) $(DEFINES) $*

.asm.o:
	casm -a $*.asm -Y -o $*.o -cs -i V:include


flipper.ld.nostrip: $(OBJ) /support/support.lib
   bumprev 36 flipper_rev
   $(CC) $(CCF) revision.c
#   delete flipper_rev.i
   $(LN) $(LNF) FROM $(SU) revision.o LIB $(LIBS) TO $@ WITH flipper.wth
   $(LN) $@ to flipper.ld NODEBUG
   delete flipper.ld.info
   delete flipper.ld.nostrip.info

release:
   copy \#?         Other:janus/Amiga/cur/flipper
   spat "chmod 755" Other:janus/Amiga/cur/flipper/\#?
   delete           Other:janus/Amiga/cur/flipper/\#?.(o|obj|bak)

clean:
   delete \#?.(o|obj|bak)

local:
   lock sys: off
   copy flipper.ld   sys:pc/Flipper
   copy flipper.info sys:pc/Flipper.info
   lock sys: on

disk:
   copy flipper.ld   PCInstall:pc/Flipper
   copy flipper.info PCInstall:pc/Flipper.info
