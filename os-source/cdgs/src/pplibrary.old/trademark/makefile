
OBJ = obj/

INCLUDE =       -iCDTV:include/ -iV37:include/ -iV37:inc.lattice -iinclude: 

CFLAGS =        -cimsq -ml -v -. -O -ms -d1 -dRELEASE=1 -o$(OBJ) $(INCLUDE)

AFLAGS =        -iCDTV:include/ -iV37:include/ -o$(OBJ) -iinclude: 
LFLAGS =        BATCH SMALLDATA SMALLCODE NOICONS ADDSYM

LIBRARIES =     lib:lc.lib lib:amiga13.lib

trademark:      $(OBJ)cdtm.o $(OBJ)tm_pic.o $(OBJ)tm_pic.raw $(OBJ)tm_pic.x $(OBJ)tm_data.x $(OBJ)cdtv_tm.raw $(OBJ)cdtv_tm.o

.c.o:
        assign ppl: /
        lc $(CFLAGS) $*.c

.a.o:
        assign ppl: /
        asm $(AFLAGS) $*.a 

.asm.o:
        assign ppl: /
        casm -a $*.asm -o $(OBJ)$*.o $(AFLAGS) 

.pic.o:
        @enforcer off >NIL:
        @enforcer quiet >NIL:
        sbox -x $*.pic ram:$*.x
        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
        delete ram:$*.x
        @enforcer >NIL:


$(OBJ)cdtv_tm.o: $(OBJ)tm_pic.x $(OBJ)tm_data.x $(OBJ)cdtv_tm.raw
        join $(OBJ)tm_data.x $(OBJ)tm_pic.x to ram:temp.x
        d2o ram:temp.x _cdtv_tm $(OBJ)cdtv_tm.o -h "_NOMERGE" 
        delete ram:temp.x

$(OBJ)cdtv_tm.raw: tm.data $(OBJ)tm_pic.raw
        join tm.data $(OBJ)tm_pic.raw as $(OBJ)cdtv_tm.raw

$(OBJ)tm_data.x: tm.data
        @enforcer off >NIL:
        @enforcer quiet >NIL:
        sbox -x tm.data $(OBJ)tm_data.x
        @enforcer >NIL:

$(OBJ)tm_pic.x: $(OBJ)tm_pic.raw
        @enforcer off >NIL:
        @enforcer quiet >NIL:
        sbox -x $(OBJ)tm_pic.raw $(OBJ)tm_pic.x
        @enforcer >NIL:

$(OBJ)tm_pic.raw: tm_pic.pic
        ilbm2raw tm_pic.pic
        copy tm_pic.pic.lo2 $(OBJ)tm_pic.raw
        delete tm_pic.pic.lo2

$(OBJ)cdtm.o:           cdtm.c /basicio/viewmgr.h /screensaver/screensaver.h /playerprefsbase.h

$(OBJ)tm_pic.o:         tm_pic.pic

