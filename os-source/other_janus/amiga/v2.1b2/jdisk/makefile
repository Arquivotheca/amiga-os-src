SU   = 
OBJ = abs.obj init.obj dev.obj beginio.obj cmds.obj badblocks.obj endcode.obj
       
CC   = lc
CCF  = -v -d4

LN   = blink
LNF  = SC SD 
LIBS = LIB:amiga.lib+LIB:janus.lib+lib:debug.lib

jdisk.device.ld.nostrip: $(OBJ)
   bumprev 36 jdisk.device_rev
   $(CC) $(CCF)revision.o revision.c
   delete jdisk.device_rev.i
   $(LN) $(LNF) FROM $(SU) revision.o LIB $(LIBS) TO $@ WITH jdisk.device.wth
   $(LN) $@ to jdisk.device.ld NODEBUG
   delete jdisk.device.ld.info
   delete jdisk.device.ld.nostrip.info

.c.o :
   $(CC) $(CCF) $*

.asm.obj:
	casm -a $*.asm -Y -o $*.obj -cs -i work:lc/include

release:
   copy \#?         Other:janus/Amiga/cur/jdisk
   spat "chmod 755" Other:janus/Amiga/cur/jdisk/\#?
   delete           Other:janus/Amiga/cur/jdisk/\#?.(o|obj|bak)

clean:
   delete \#?.(o|obj|bak)

local:
   lock sys: off
   copy jdisk.device.ld   devs:JDisk.device
   lock sys: on

disk:
   copy jdisk.device.ld   PCInstall:devs/JDisk.device
