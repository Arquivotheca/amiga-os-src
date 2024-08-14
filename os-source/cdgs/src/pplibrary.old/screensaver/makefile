
OBJ = obj/

INCLUDE =       -iCDTV:include/ -iV37:include/ -iV37:inc.lattice -iinclude: 

CFLAGS =        -cimsq -ml -v -. -O -ms -d1 -dRELEASE=1 -o$(OBJ) $(INCLUDE)

AFLAGS =        -iCDTV:include/ -iV37:include/ -o$(OBJ) -iinclude: 
LFLAGS =        BATCH SMALLDATA SMALLCODE NOICONS ADDSYM

LIBRARIES =     lib:lc.lib lib:amiga13.lib # lib:debug.lib


screensaver:    $(OBJ)splinesaver.o $(OBJ)screensaver.o $(OBJ)splines.o $(OBJ)rnd.o


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



$(OBJ)splinesaver.o:    splinesaver.c splinesaver.h /playerprefsbase.h
 
$(OBJ)screensaver.o:    screensaver.c screensaver.h splinesaver.h /playerprefsbase.h

$(OBJ)splines.o:        splines.a splinesaver.h

$(OBJ)rnd.o:            rnd.a

