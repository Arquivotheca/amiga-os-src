SU   = lib:c.o
OBJ  = main.o display.o disk.o janus.o change.o stringutils.o debug.o inhibit.o startup.o

DEFINES = -DWB_SUPPORT=1 
       
CC   = lc
CCF  = -v -d4 -b0

LN   = blink
LNF  = SC SD DEFINE __main=__tinymain
LIBS = LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib+lib:debug.lib

.c.o :
   $(CC) $(CCF) $(DEFINES) $*

.asm.o:
	casm -a $*.asm -Y -o $*.o -cs -i work:lc/include


flipper.ld.nostrip: $(OBJ)
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
