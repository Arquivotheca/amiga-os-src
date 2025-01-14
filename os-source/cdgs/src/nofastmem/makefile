
# makefile for smake

LOCAL = gs:nofastmem/

INCLUDE =    IDIR=cdgs:include/ IDIR=V39:include/ IDIR=V39:inc.lattice IDIR=include: 
OBJ = obj/
LST = lst/

CFLAGS =     NMINC MCCONS STRMERGE STREQ NOSTKCHK NOVERSION OPTIMIZE OPTSIZE DEBUG=L CPU=68020 OBJNAME=obj/ \
             $(INCLUDE) GST t:all.gst

AFLAGS =    -icdgs:include/ -iV39:include/ -iinclude: -s

.c.o:
        sc $(CFLAGS) $*.c

.asm.o:
        hx68 -a $*.asm -o $(OBJ)$*.o $(AFLAGS) 

nofastmem.ld:   $(OBJ)nofastmem.o
                    slink LIBRARY lib:sc.lib lib:amiga.lib lib:debug.lib TO nofastmem.ld \
                    FROM obj/nofastmem.o \
                    VERBOSE SMALLCODE SMALLDATA addsym ND

pragmas:
         @echo "Making pragmas/protos"
         @execute < <
         failat 21 ;
         makedir ram:sfd ;
         copy nofastmem_lib.sfd ram:sfd ;
         cd ram:sfd ;
         sfd nofastmem_lib.sfd ;
         copy PROTOS/\#? TO $(LOCAL) QUIET;
         copy PRAGMAS/\#? TO $(LOCAL) QUIET;
         copy FD/\#? TO $(LOCAL) QUIET;
         cd ram: ;
         delete ram:sfd/\#? ALL FORCE QUIET;
         delete ram:sfd QUIET ;
         <

autodoc:
         autodoc >nofastmem.doc nofastmem.asm

bumprev:
         bumprev 40 nofastmem_rev

release:
         smake bumprev
         smake
         smake pragmas
         smake autodoc

$(OBJ)nofastmem.o:      nofastmem.asm nofastmem_rev.i

