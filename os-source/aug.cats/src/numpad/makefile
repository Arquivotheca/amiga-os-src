
   /***********************************************************************
   *                                                                      *
   *                            COPYRIGHTS                                *
   *                                                                      *
   *   UNLESS OTHERWISE NOTED, ALL FILES ARE                              *
   *   Copyright (c) 1990  Commodore-Amiga, Inc.  All Rights Reserved.    *
   *                                                                      *
   ***********************************************************************/

# NATIVE BUILD ONLY

APP=numpad
SU=lib:c.o
CC=lc
CCF=-csf -v -iinclude: -o
LN=blink
LNF=NODEBUG SMALLCODE SMALLDATA DEFINE __main=__tinymain
#LIBS=lib:lc.lib+V36:devsupport/linker_libs/amiga.lib+lib:debug.lib
LIBS=lib:lc.lib+V36:devsupport/linker_libs/amiga.lib

INC=local.h app.h
OBJ=main.o cx.o window.o app.o $(APP).o

$(APP).ld: $(OBJ)
   bumprev 37 $(APP)_rev
   $(CC) $(CCF)revision.o revision.c
   delete $(APP)_rev.i
   $(LN) $(LNF) FROM $(SU) $(OBJ) revision.o LIB $(LIBS) TO $@

main.o : main.c $(INC)
   $(CC) $(CCF)$@ main.c

cx.o : cx.c $(INC)
   $(CC) $(CCF)$@ cx.c

window.o : window.c $(INC)
   $(CC) $(CCF)$@ window.c

app.o : app.c app.h
   $(CC) $(CCF)$@ app.c

$(APP).o : $(APP).c app.h
   $(CC) $(CCF)$@ $(APP).c
