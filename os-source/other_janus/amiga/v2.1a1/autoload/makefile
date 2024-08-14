SU       =  lib:c.o
OBJ      =  autoload.o
LIBS     =  LIB:lc.lib+LIB:janus.lib+LIB:amiga.lib+lib:debug.lib
       
CC       =  lc
CCF      = -csf -v -d4 -b0 -q500w1e 
AS       = assem
ASF      = -c w200000 -i fh1:languages/latticec5.02/lc/include -o
LN       = blink
LNF      = SMALLCODE SMALLDATA DEFINE __main=__tinymain

autoload.ld.nostrip: $(OBJ)
   bumprev 36 autoload_rev
   $(CC) $(CCF)revision.o revision.c
   delete autoload_rev.i
   $(LN) $(LNF) FROM $(SU) $(OBJ) revision.o LIB $(LIBS) TO $@
   $(LN) $@ to autoload.ld NODEBUG
   delete autoload.ld.info
   delete autoload.ld.nostrip.info

autoload.o :   autoload.c
   $(CC) $(CCF) autoload.c

clean:
   delete \#?.obj
   delete \#?.o
   delete \#?.bak

local:
   lock sys: off
   copy autoload.ld sys:pc/services/AutoLoad
   lock sys: on

disk:
   copy autoload.ld PCInstall:pc/services/AutoLoad

release:
   copy \#?         Other:janus/Amiga/cur/autoload
   spat "chmod 755" Other:janus/Amiga/cur/autoload/\#?
   delete           Other:janus/Amiga/cur/autoload/\#?.o
   delete           Other:janus/Amiga/cur/autoload/\#?.obj
   delete           Other:janus/Amiga/cur/autoload/\#?.bak
