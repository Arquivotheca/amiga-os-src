SU   = lib:c.o
OBJ  = setprefs.obj mainprog.obj gadgets.obj
       
CC   = lc
CCF  = -v -d4

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

setprefs.obj :   setprefs.asm mydata.i
   casm -a setprefs.asm -y -o setprefs.obj -cs -i work:lc/include

mainprog.obj :   mainprog.asm mydata.i
   casm -a mainprog.asm -y -o mainprog.obj -cs -i work:lc/include

gadgets.obj :   gadgets.asm
   casm -a gadgets.asm -y -o gadgets.obj -cs -i work:lc/include

.c.o :
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
