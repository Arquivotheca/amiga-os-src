
OBJ = obj/
CFLAGS =  NMINC MCCONS STRMERGE STREQ NOSTKCHK NOVERSION OPTIMIZE OPTSIZE DEBUG=L GST t:all.gst OBJNAME=$(OBJ) $(INCLUDE)
LIBRARIES =     lib:sc.lib lib:amiga.lib lib:debug.lib

freeanim.ld: $(OBJ)main.o

.c.o:
    sc $(CFLAGS) $*
    slink FROM lib:c.o obj/$*.o to freeanim.ld LIB $(LIBRARIES) ND
    copy freeanim.ld freeanim

$(OBJ)main.o: main.c

