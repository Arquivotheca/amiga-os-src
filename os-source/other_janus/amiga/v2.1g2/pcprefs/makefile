SU   = lib:c.o
OBJ  = setprefs.o mainprog.o gadgets.o
       
CC   = lc
CCF  = -v -d4 -b0

LN   = blink
LNF  = SMALLCODE SMALLDATA DEFINE __main=__tinymain
LIBS = lib:lc.lib+LIB:amiga.lib+LIB:janus.lib+lib:debug.lib

pcprefs.ld.nostrip: $(OBJ)
   bumprev 36 pcprefs_rev
   $(CC) $(CCF) revision.c
   delete pcprefs_rev.i
   $(LN) $(LNF) FROM $(SU) $(OBJ) revision.o LIB $(LIBS) TO $@
   $(LN) $@ to pcprefs.ld NODEBUG
   delete pcprefs.ld.info
   delete pcprefs.ld.nostrip.info

.c.o:
   $(CC) $(CCF) $*

setprefs.o: setprefs.c mydata.h
	$(CC) $(CCF) $*

mainprog.o: mainprog.c mydata.h
	$(CC) $(CCF) $*

gadgets.o:  gadgets.c
	$(CC) $(CCF) $*

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
