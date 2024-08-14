
# makefile for smake

OBJ    =  obj/
LST    =  lst/
FLAGS  =  -s -i include:
CFLAGS =  NMINC MCCONS STRMERGE STREQ NOSTKCHK NOVERSION OPTIMIZE OPTSIZE DEBUG=L GST t:all.gst OBJNAME=$(OBJ)
LIBRARIES = lib:sc.lib lib:amiga.lib lib:debug.lib

setpatchcd.ld: $(OBJ)main.o 
               slink lib:c.o obj/main.o TO setpatchcd.ld LIB $(LIBRARIES) VERBOSE SMALLCODE SMALLDATA ND


autodoc:
        autodoc >cd.doc init.asm task.asm

.c.o:
        sc $(CFLAGS) $*

.asm.o:
        hx68 -a $*.asm -o $(OBJ)$*.o $(FLAGS)

bumprev:
         bumprev 40 setpatchcd_rev

release:
         smake bumprev
         smake
         smake autodoc


$(OBJ)main.o: main.c setpatchcd_rev.h
