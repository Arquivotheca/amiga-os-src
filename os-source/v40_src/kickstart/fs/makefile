# NATIVE BUILD ONLY

.SUFFIXES: .asm .o

LINKER  = blink
ASM	= casm
V = 40

SRC = readwrite.asm rwffs.asm bitmap.asm disk.asm bufalloc.asm queues.asm\
      colib.asm qpkt.asm taskwait.asm support.asm init.asm restart.asm\
      work.asm exinfo.asm errors.asm rlocking.asm notify.asm main.asm\
      validator.asm rwofs.asm dirlock.asm

OBJECTS = $(OBJ)main.o $(OBJ)readwrite.o $(OBJ)rwffs.o $(OBJ)bitmap.o \
	  $(OBJ)bufalloc.o $(OBJ)queues.o $(OBJ)colib.o $(OBJ)qpkt.o \
	  $(OBJ)taskwait.o $(OBJ)support.o $(OBJ)init.o $(OBJ)restart.o \
	  $(OBJ)work.o $(OBJ)exinfo.o $(OBJ)errors.o $(OBJ)rlocking.o \
	  $(OBJ)notify.o $(OBJ)disk.o $(OBJ)validator.o $(OBJ)rwofs.o \
	  $(OBJ)dirlock.o

.asm.o:
	$(ASM) -A $*.asm -o$@ -iinclude: -c=QS -h$(PROGRAM).i

all:	fs ffs

fs:
	lmk $@.ld.strip OBJ=$@_obj/ PROGRAM=$@

ffs:
	lmk $@.ld.strip OBJ=$@_obj/ PROGRAM=$@

$(PROGRAM).ld:  $(OBJECTS)
	$(LINKER) <WITH <
FROM	$(OBJECTS)
TO	$(PROGRAM).ld
LIBRARY lib:debug.lib debug.lib lib:amiga.lib
<

$(PROGRAM).ld.strip: $(PROGRAM).ld
	$(LINKER) $(PROGRAM).ld to $(PROGRAM).ld.strip ND

$(OBJ)main.o       : main.asm globals.i actions.i private.i fs_rev.i
$(OBJ)init.o       : init.asm globals.i moredos.i private.i
$(OBJ)qpkt.o       : qpkt.asm globals.i
$(OBJ)taskwait.o   : taskwait.asm globals.i
$(OBJ)colib.o      : colib.asm globals.i private.i
$(OBJ)disk.o       : disk.asm globals.i private.i moredos.i
$(OBJ)bufalloc.o   : bufalloc.asm globals.i private.i
$(OBJ)support.o    : support.asm globals.i moredos.i private.i
$(OBJ)work.o       : work.asm globals.i private.i moredos.i
$(OBJ)exinfo.o     : exinfo.asm globals.i private.i moredos.i
$(OBJ)restart.o    : restart.asm globals.i private.i moredos.i
$(OBJ)bitmap.o     : bitmap.asm globals.i moredos.i private.i
$(OBJ)queues.o     : queues.asm globals.i private.i moredos.i
$(OBJ)validator.o  : validator.asm globals.i private.i moredos.i
$(OBJ)errors.o     : errors.asm globals.i private.i moredos.i
$(OBJ)rlocking.o   : rlocking.asm globals.i private.i moredos.i
$(OBJ)notify.o     : notify.asm globals.i private.i moredos.i
$(OBJ)readwrite.o  : readwrite.asm globals.i readwrite.i moredos.i private.i
$(OBJ)rwffs.o      : rwffs.asm globals.i readwrite.i moredos.i private.i
$(OBJ)rwofs.o      : rwofs.asm globals.i readwrite.i moredos.i private.i
$(OBJ)dirlock.o    : dirlock.asm globals.i private.i

version:
	bumprev $(V) fs_rev
	bumprev $(V) ffs_rev

backup:
	copy \#?.asm \#?.i \#?rev\#? makefile  vax:fs all nopro

internal:
	lmk
	copy (\#?.ld\#?|\#?.map|\#?_rev.\#?) to ks:fs quiet nopro
	rsh heartofgold -l jesup "cd /usr/commodore/amiga/V$(V)/src/kickstart/fs; co RCS/**"
	execute setresourcerev $(V)
	echo "*N	Please rebuild filesysres!"
