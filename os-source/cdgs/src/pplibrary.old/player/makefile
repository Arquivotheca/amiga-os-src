
OBJ = obj/

CFLAGS =        -cimsq -ml -ms -. -O -v -d1 -o$(OBJ) $(INCLUDE)

AFLAGS =        -iCDTV:include/ -iV37:include/ -o$(OBJ) -iinclude: 

player:         $(OBJ)cdg.o $(OBJ)gadgets.o $(OBJ)player.o \
                $(OBJ)playerstrip.o $(OBJ)scrdata.o $(OBJ)scrbuttons.o $(OBJ)cdgnumb.o
                blink with player.lnk

.c.o:
        assign ppl: /
        lc $(CFLAGS) $*.c

.pic.o:
        @enforcer off >NIL:
        @enforcer quiet >NIL:
        sbox -x $*.pic ram:$*.x
        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
        delete ram:$*.x
        @enforcer >NIL:



$(OBJ)player.o:         player.c /cdtvkeys.h /playerprefs.h /playerprefsbase.h /basicio/keyclick.h

$(OBJ)cdg.o:            cdg.c /playerprefs.h /playerprefsbase.h

$(OBJ)gadgets.o:        gadgets.c /cdtvkeys.h /playerprefs.h

$(OBJ)playerstrip.o:    playerstrip.pic

$(OBJ)scrdata.o:        scrdata.pic

$(OBJ)scrbuttons.o:     scrbuttons.pic
                        @enforcer off >NIL:
                        @enforcer quiet >NIL:
                        sbox -x -M $*.pic ram:$*.x
                        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
                        delete ram:$*.x
                        @enforcer >NIL:

$(OBJ)cdgnumb.o:        cdgnumb.pic

