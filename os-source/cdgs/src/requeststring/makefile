
OBJ = obj/

INCLUDE = IDIR=V39:include/ IDIR=include:

CFLAGS =  NMINC MCCONS STRMERGE STREQ NOSTKCHK NOVERSION OPTIMIZE OPTSIZE DEBUG=L GST t:all.gst OBJNAME=$(OBJ) $(INCLUDE)
AFLAGS =        -icdgs:include/ -iV39:include/ -iinclude: 
LFLAGS =        BATCH SMALLDATA SMALLCODE NOICONS ADDSYM

LIBRARIES =     lib:sc.lib lib:amiga.lib lib:debug.lib


test.ld:   $(OBJ)requeststring.o

.c.o:
    sc $(CFLAGS) $*
    slink FROM lib:c.o obj/$*.o to $* LIB $(LIBRARIES) ND
    copy $* c:RequestString

.pic.o:
        @enforcer off >NIL:
        @enforcer quiet >NIL:
        sbox -x $*.pic ram:$*.x
        d2o ram:$*.x _$* $(OBJ)$*.o -h "_NOMERGE"
        delete ram:$*.x
        @enforcer >NIL:

$(OBJ)requeststring.o:           requeststring.c

