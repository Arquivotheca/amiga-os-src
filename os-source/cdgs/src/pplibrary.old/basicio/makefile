
OBJ = obj/

INCLUDE =       -iCDTV:include/ -iV37:include/ -iV37:inc.lattice -iinclude: 

CFLAGS =        -cimsq -ml -v -. -O -ms -d1 -dRELEASE=1 -o$(OBJ) $(INCLUDE)

AFLAGS =        -iCDTV:include/ -iV37:include/ -o$(OBJ) -iinclude: 
LFLAGS =        BATCH SMALLDATA SMALLCODE NOICONS ADDSYM

LIBRARIES =     lib:lc.lib lib:amiga13.lib # lib:debug.lib

basicio:        $(OBJ)amigafunc.o $(OBJ)bookmark.o $(OBJ)clicker.o $(OBJ)joy.o $(OBJ)libinit.o $(OBJ)screen.o \
                $(OBJ)routines.o $(OBJ)viewmgr.o \
                $(OBJ)rock.o $(OBJ)ray.o $(OBJ)bstrip.o


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



$(OBJ)amigafunc.o:      amigafunc.c

$(OBJ)bookmark.o:       bookmark.c /playerprefsbase.h /screensaver/screensaver.h keyclick.h

$(OBJ)clicker.o:        clicker.c /playerprefsbase.h keyclick.h

$(OBJ)joy.o:            joy.c

$(OBJ)libinit.o:        libinit.c /playerprefsbase.h /playerprefs_rev.h
                        lc $(CFLAGS) -b0 libinit.c

$(OBJ)screen.o:         screen.c /playerprefsbase.h viewmgr.h /screensaver/screensaver.h

$(OBJ)routines.o:       routines.a /playerprefs.i

$(OBJ)viewmgr.o:        viewmgr.a viewmgr.i /playerprefs_func.i

$(OBJ)rock.o:           rock.pic

$(OBJ)ray.o:            ray.pic
                        @enforcer off >NIL:
                        @enforcer quiet >NIL:
                        sbox -C -R $*.pic ram:$*.x
                        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
                        delete ram:$*.x
                        @enforcer >NIL:

$(OBJ)bstrip.o:         bstrip.pic
                        @enforcer off >NIL:
                        @enforcer quiet >NIL:
                        sbox -C -R $*.pic ram:$*.x
                        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
                        delete ram:$*.x
                        @enforcer >NIL:

