
OBJ = obj/

INCLUDE =       -iCDTV:include/ -iV37:include/ -iV37:inc.lattice -iinclude: 

CFLAGS =        -cimsq -ml -v -. -O -ms -d1 -dRELEASE=1 -o$(OBJ) $(INCLUDE)

AFLAGS =        -iCDTV:include/ -iV37:include/ -o$(OBJ) -iinclude: 
LFLAGS =        BATCH SMALLDATA SMALLCODE NOICONS ADDSYM

LIBRARIES =     lib:lc.lib lib:amiga13.lib lib:debug.lib


prefs:          $(OBJ)prefs.o $(OBJ)prefsgadgets.o $(OBJ)creditcard.o $(OBJ)time.o $(OBJ)language.o $(OBJ)screen.o\
                $(OBJ)prefsstrip.o $(OBJ)prefsdata_cr.o $(OBJ)prefsbuttons_cr.o $(OBJ)prefsasm.o
                blink with prefs.lnk LIB $(LIBRARIES)

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



$(OBJ)prefs.o:                  prefs.c /basicio/keyclick.h /playerprefs.h /playerprefsbase.h 

$(OBJ)prefsgadgets.o:           prefsgadgets.c /playerprefs.h

$(OBJ)creditcard.o:             creditcard.c /playerprefs.h

$(OBJ)time.o:                   time.c /playerprefs.h

$(OBJ)language.o:               language.c /playerprefs.h /playerprefsbase.h

$(OBJ)screen.o:                 screen.c /playerprefs.h /playerprefsbase.h

$(OBJ)prefsstrip.o:             prefsstrip.pic

$(OBJ)prefsdata_cr.o:           prefsdata_cr.pic

$(OBJ)prefsbuttons_cr.o:        prefsbuttons_cr.pic
                                @enforcer off >NIL:
                                @enforcer quiet >NIL:
                                sbox -x -M $*.pic ram:$*.x
                                d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
                                delete ram:$*.x
                                @enforcer >NIL:

$(OBJ)prefsasm.o:               prefsasm.a /playerprefs.i
