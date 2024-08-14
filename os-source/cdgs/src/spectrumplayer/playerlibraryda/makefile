# makefile for smake

SRCDIR= CDGS:src/playerlibrary
LOCAL = work:src/playerlibrary/

CFILES= cd.c vfd.c main.c task.c playlist.c
AFILES= start.asm endcode.asm
HFILES= playerlibrary.h defs.h vfd_funcs.h task_funcs.h cd_funcs.h \
    playlist_funcs.h playerlibrary_rev.h
IFILES=     playerlibrary.i internal.i playerlibrary_rev.i
MISCFILES=  makefile player_lib.sfd player_lib.fd playerlibrary.lnk \
        playerlibrary_rev.rev playerlibrary.ld
EXTERNHIFILES=  playerlibrary.h playerlibrary.i

INCLUDE =    IDIR=cdgs:include/ IDIR=V39:include/ IDIR=V39:inc.lattice IDIR=include: 
OBJ = obj/
LST = lst/

CFLAGS =     NMINC MCCONS STRMERGE STREQ NOSTKCHK NOVERSION OPTIMIZE OPTSIZE DEBUG=L CPU=68020 OBJNAME=obj/ \
             $(INCLUDE) GST t:all.gst

AFLAGS =  -s -i include:

.c.o:
        sc $(CFLAGS) $*.c

.asm.o:
        hx68 -a $*.asm -o $(OBJ)$*.o $(AFLAGS) 

playerlibrary.ld:   $(OBJ)start.o $(OBJ)main.o $(OBJ)task.o $(OBJ)cd.o $(OBJ)vfd.o $(OBJ)playlist.o $(OBJ)endcode.o \
                    $(OBJ)analyze.o $(OBJ)print.o
                    slink LIBRARY lib:sc.lib lib:amiga.lib lib:debug.lib TO playerlibrary.ld \
                    FROM obj/start.o obj/main.o obj/task.o obj/cd.o obj/vfd.o obj/playlist.o \
                    obj/analyze.o obj/print.o obj/endcode.o \
                    VERBOSE SMALLCODE SMALLDATA addsym ND
                    copy playerlibrary.ld libs:player.library
                    copy playerlibrary.ld df0:

pragmas:
         @echo "Making pragmas/protos"
         @execute < <
         failat 21 ;
         makedir ram:sfd ;
         copy player_lib.sfd ram:sfd ;
         cd ram:sfd ;
         sfd player_lib.sfd ;
         copy PROTOS/\#? TO $(LOCAL) QUIET;
         copy PRAGMAS/\#? TO $(LOCAL) QUIET;
         copy FD/\#? TO $(LOCAL) QUIET;
         cd ram: ;
         delete ram:sfd/\#? ALL FORCE QUIET;
         delete ram:sfd QUIET ;
         <

autodoc:
         autodoc >playerlibrary.doc main.c analyze.asm

bumprev:
         bumprev 40 playerlibrary_rev

release:
         smake bumprev
         smake
         smake pragmas
         smake autodoc

internal:
    copy $(CFILES) $(SRCDIR) nopro
    copy $(AFILES) $(SRCDIR) nopro
    copy $(HFILES) $(SRCDIR) nopro
    copy $(IFILES) $(SRCDIR) nopro
    copy $(MISCFILES) $(SRCDIR) nopro
    copy $(EXTERNHIFILES) CDGS:include/internal nopro


$(OBJ)start.o:      start.asm playerlibrary_rev.i internal.i playerlibrary.i

$(OBJ)endcode.o:    endcode.asm playerlibrary_rev.i internal.i playerlibrary.i

$(OBJ)main.o:       main.c playerlibrary.h defs.h

$(OBJ)task.o:       task.c playerlibrary.h defs.h

$(OBJ)cd.o:         cd.c playerlibrary.h defs.h

$(OBJ)vfd.o:        vfd.c playerlibrary.h defs.h

$(OBJ)playlist.o:   playlist.c playerlibrary.h defs.h

$(OBJ)analyze.o:    analyze.asm

$(OBJ)print.o:      print.asm

