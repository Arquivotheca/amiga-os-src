
OBJ = obj/

INCLUDE =   IDIR=v39:inc.lattice/ IDIR=V39:include/ IDIR=CDGS:include/ IDIR=include:

CFLAGS =    NMINC MCCONS STRMERGE STREQ NOSTKCHK NOVERSION OPTIMIZE OPTSIZE CPU=68020 DEBUG=L OBJNAME=$(OBJ) \
            $(INCLUDE) GST t:all.gst
AFLAGS =    -iCDGS:include/ -iV39:include/ -iinclude: 
LFLAGS =    BATCH SMALLDATA SMALLCODE NOICONS ADDSYM

LIBRARIES = lib:sc.lib lib:amiga.lib lib:debug.lib


cdstrap.ld: $(OBJ)start.o $(OBJ)main.o $(OBJ)functions.o $(OBJ)endcode.o \
            $(OBJ)trademark.o $(OBJ)cdtvtm.o $(OBJ)strapimage.o $(OBJ)animation.o
            slink LIBPREFIX _LIB BATCH SMALLCODE NOICONS LIB $(LIBRARIES) \
                  TO cdstrap.ld FROM obj/start.o obj/main.o obj/functions.o obj/trademark.o obj/cdtvtm.o \
                  obj/endcode.o

bumprev:
         bumprev 40 cdstrap_rev

release:
         smake bumprev
         smake

.c.o:
        sc $(CFLAGS) $*.c

.a.o:
        asm $(AFLAGS) $*.a 

.asm.o:
        casm -a $*.asm -o $(OBJ)$*.o $(AFLAGS) 

.pic.o:
        @enforcer off >NIL:
        @enforcer quiet >NIL:
        sbox -x $*.pic ram:$*.x
        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
        delete ram:$*.x
        @enforcer >NIL:

.data.o:
        @enforcer off >NIL:
        @enforcer quiet >NIL:
        sbox -x $*.data ram:$*.x
        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
        delete ram:$*.x
        @enforcer >NIL:




$(OBJ)trademark.o:      trademark.data

$(OBJ)cdtvtm.o:         cdtvtm.data

$(OBJ)main.o:           main.c gs:cd/cd.h cdstrap.h

$(OBJ)functions.o:      functions.c gs:cd/cd.h cdstrap.h

$(OBJ)start.o:          start.asm cdstrap_rev.i

$(OBJ)endcode.o:        endcode.asm
