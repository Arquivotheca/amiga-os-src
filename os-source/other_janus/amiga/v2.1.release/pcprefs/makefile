SU   = V:lib/lattice/c.o
OBJ  = pcprefs.o 
       
CC   = lc
CCF  = -v -cat -b0 -HV:include/all.ph -iV:include -iV:inc.lattice

LN   = blink
LNF  = SMALLCODE SMALLDATA DEFINE __main=__tinymain
LIBS = V:lib/lattice/lc.lib+V:lib/amiga.lib+V:LIB/janus.lib+/support/support.lib

pcprefs.ld.nostrip: $(OBJ) /support/support.lib
   bumprev 36 pcprefs_rev
   $(CC) $(CCF) revision.c
   delete pcprefs_rev.i
   $(LN) $(LNF) FROM $(SU) $(OBJ) revision.o LIB $(LIBS) TO $@
   $(LN) $@ to pcprefs.ld NODEBUG
   delete pcprefs.ld.info
   delete pcprefs.ld.nostrip.info

.c.o:
   $(CC) $(CCF) $*

pcprefs.o: pcprefs.c

release:
   copy \#?         Other:janus/Amiga/cur/pcprefs
   spat "chmod 755" Other:janus/Amiga/cur/pcprefs/\#?
   delete           Other:janus/Amiga/cur/pcprefs/\#?.(o|obj|bak)

clean:
   delete \#?.(o|obj|bak)

local:
   lock sys: off
   copy pcprefs.ld   sys:pc/PCPrefs
   copy pcprefs.info sys:pc/PCPrefs.info
   lock sys: on

disk:
   copy pcprefs.ld   PCInstall:pc/PCPrefs
   copy pcprefs.info PCInstall:pc/PCPrefs.info
