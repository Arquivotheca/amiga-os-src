
HDRS = ram.h ram_includes.h block_types.h protos.h
OBJ = dobj/
LC = lc
ASM = asm
PRECOMP = -cfims -v -O -ms -rr
#LCFLAGS = -cfims -v -O -ms -rr -Hincludes.precomp -o$(OBJ)
LCFLAGS = -cfims -v -rr -d3 -o$(OBJ)
ASMFLAGS = -iINCLUDE: -o$(OBJ)
LIB = lib:

ram-handler.db: includes.precomp \
	     $(OBJ)start.o $(OBJ)support.o $(OBJ)support2.o \
	     $(OBJ)checkname.o $(OBJ)comment.o $(OBJ)create.o $(OBJ)delete.o \
	     $(OBJ)deletedir.o $(OBJ)diskinfo.o $(OBJ)adddevice.o \
	     $(OBJ)exall.o $(OBJ)file.o $(OBJ)finddir.o $(OBJ)findentry.o \
	     $(OBJ)locate.o $(OBJ)lock.o $(OBJ)mygetvec.o $(OBJ)closefile.o \
	     $(OBJ)parent.o $(OBJ)rename.o $(OBJ)renamedisk.o $(OBJ)seek.o \
	     $(OBJ)transfer.o $(OBJ)global.o $(OBJ)ram_c.o \
	     $(OBJ)notify.o $(OBJ)dbsupp.o
	blink with ramd.lnk
.c.o:
	$(LC) $(LCFLAGS) $*

.c.precomp:
	$(LC) -ph $(PRECOMP) $*
	copy ram:$*.q $*.precomp

.a.o:
	$(ASM) $(ASMFLAGS) $*

includes.precomp: includes.c $(HDRS)

$(OBJ)dbsupp.o: dbsupp.c

$(OBJ)start.o: start.c includes.precomp

$(OBJ)checkname.o: checkname.c includes.precomp

$(OBJ)comment.o: comment.c includes.precomp

$(OBJ)create.o: create.c includes.precomp

$(OBJ)delete.o: delete.c includes.precomp

$(OBJ)deletedir.o: deletedir.c includes.precomp

$(OBJ)diskinfo.o: diskinfo.c includes.precomp

$(OBJ)exall.o: exall.c includes.precomp

$(OBJ)file.o: file.c includes.precomp

$(OBJ)finddir.o: finddir.c includes.precomp

$(OBJ)findentry.o: findentry.c includes.precomp

$(OBJ)locate.o: locate.c includes.precomp

$(OBJ)lock.o: lock.c includes.precomp

$(OBJ)mygetvec.o: mygetvec.c includes.precomp

$(OBJ)parent.o: parent.c includes.precomp

$(OBJ)rename.o: rename.c includes.precomp

$(OBJ)renamedisk.o: renamedisk.c includes.precomp

$(OBJ)seek.o: seek.c includes.precomp

$(OBJ)transfer.o: transfer.c includes.precomp

$(OBJ)global.o: global.c includes.precomp

$(OBJ)adddevice.o: adddevice.c includes.precomp

$(OBJ)closefile.o: closefile.c includes.precomp

$(OBJ)notify.o: notify.c includes.precomp /prefs.h

$(OBJ)ram_c.o: ram_c.a

$(OBJ)support.o: support.a

$(OBJ)support2.o: support2.a

clean:
	delete $(OBJ)\#?.o

backup:
	copy g:prefs/ram sun:prefs/c all
