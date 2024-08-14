
OBJ = obj/

INCLUDE = IDIR=V39:include/ IDIR=include:

CFLAGS =  NMINC MCCONS STRMERGE STREQ NOSTKCHK NOVERSION OPTIMIZE OPTSIZE DEBUG=L GST t:all.gst OBJNAME=$(OBJ) $(INCLUDE)
AFLAGS =        -icdgs:include/ -iV39:include/ -iinclude: 
LFLAGS =        BATCH SMALLDATA SMALLCODE NOICONS ADDSYM

LIBRARIES =     lib:sc.lib lib:amiga.lib lib:debug.lib


test.ld:   $(OBJ)CDPlayer.o $(OBJ)gadgets.o $(OBJ)tgadgets.o
    slink FROM lib:c.o $(OBJ)CDPlayer.o $(OBJ)gadgets.o $(OBJ)tgadgets.o to CDPlayer.ld LIB $(LIBRARIES) ND
    copy CDPlayer.ld CDPlayer

.c.o:
    sc $(CFLAGS) $*

.pic.o:
        @enforcer off >NIL:
        @enforcer quiet >NIL:
        sbox -x $*.pic ram:$*.x
        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
        delete ram:$*.x
        @enforcer >NIL:

bumprev:
        bumprev 40 cdplayer_rev

$(OBJ)CDPlayer.o:         CDPlayer.c cdplayer_rev.h

$(OBJ)gadgets.o:          gadgets.c cdplayer_rev.h

$(OBJ)Tgadgets.o:         Tgadgets.c cdplayer_rev.h

