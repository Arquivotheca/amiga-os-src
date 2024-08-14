SU   = LIB:c.o
OBJ  = djmount.o
       
CC   = lc
CCF  = -v -d4

LN   = blink
LNF  = SMALLCODE SMALLDATA DEFINE __main=__tinymain
LIBS = LIB:lc.lib+LIB:amiga.lib+LIB:janus.lib+lib:debug.lib

djmount.ld.nostrip: $(OBJ)
   bumprev 36 djmount_rev
   $(CC) $(CCF)revision.o revision.c
   delete djmount_rev.i
   $(LN) $(LNF) FROM $(SU) $(OBJ) revision.o LIB $(LIBS) TO $@
   $(LN) $@ to djmount.ld NODEBUG
   delete djmount.ld.info
   delete djmount.ld.nostrip.info

.c.o :
   $(CC) $(CCF) $*

release:
   copy \#?         Other:janus/Amiga/cur/djmount
   spat "chmod 755" Other:janus/Amiga/cur/djmount/\#?
   delete           Other:janus/Amiga/cur/djmount/\#?.o
   delete           Other:janus/Amiga/cur/djmount/\#?.obj
   delete           Other:janus/Amiga/cur/djmount/\#?.bak

clean:
   delete \#?.o
   delete \#?.obj
   delete \#?.bak

local:
   lock sys: off
   copy djmount.ld   c:DJMount
   lock sys: on

disk:
   copy djmount.ld   PCInstall:c/DJMount
   copy djmount.ld   PCInstall:+c/DJMount
