# Makefile for A2091/A590 SCSI/XT device driver

#MAKE = lmk
MAKE = smake

.SUFFIXES: .asm .o

SOURCES = init.asm lib.asm iotask.asm scsitask.asm xttask.asm attask.asm \
	  mountstuff.asm loadseg.asm endcode.asm
OBJECTS = $(OBJ)init.o $(OBJ)lib.o $(OBJ)mountstuff.o $(OBJ)iotask.o \
	  $(OBJ)scsitask.o $(OBJ)xttask.o $(OBJ)attask.o $(OBJ)loadseg.o \
          $(OBJ)endcode.o $(OBJ)ncr.o
INCLUDES= device.i board.i modifiers.i

.asm.o:
	hx68 -A $*.asm -o$@ -icdtv:include  -iinclude: -c=QS -h$(PROGRAM).i

.c.o:
	sc $*.c OBJECTNAME=$@ DEFINE HEADER="$(PROGRAM).h"

all:	scsidisk a300 cdtv_ide a590 a590disk a1000 a3090 a3090disk a4000t \
	a3090.rom

ld_strip.ld:
	sc LINK $* PROGRAMNAME=$@ STRIPDEBUG NOMAP

scsidisk:
	$(MAKE) $@.ld_strip OBJ=$@_obj/ PROGRAM=$@

a300:
	$(MAKE) $@.ld_strip OBJ=$@_obj/ PROGRAM=$@

a1000:
	$(MAKE) $@.ld_strip OBJ=$@_obj/ PROGRAM=$@

cdtv_ide:
	$(MAKE) $@.ld_strip OBJ=$@_obj/ PROGRAM=$@

#a2091:
#	$(MAKE) $@.ld_strip OBJ=$@_obj/ PROGRAM=$@

a590:
	$(MAKE) $@.ld_strip OBJ=$@_obj/ PROGRAM=$@

#a2091disk:
#	$(MAKE) $@.ld_strip OBJ=$@_obj/ PROGRAM=$@

a590disk:
	$(MAKE) $@.ld_strip OBJ=$@_obj/ PROGRAM=$@

#a2090:
#	$(MAKE) $@.ld_strip OBJ=$@_obj/ PROGRAM=$@

a3090:
	$(MAKE) $@.ld_strip OBJ=$@_obj/ PROGRAM=$@

a3090disk:
	$(MAKE) $@.ld_strip OBJ=$@_obj/ PROGRAM=$@

a4000t:
	$(MAKE) $@.ld_strip OBJ=$@_obj/ PROGRAM=$@

a3090_load: a3090_load.o
	sc LINK $*.o PROGRAMNAME=$@
	
a3090_load.o	   : a3090_load.asm loadseg.asm modifiers.i
	hx68 -A $*.asm -o$@ -icdtv:include  -iinclude: -c=QS -ha3090_load.i

# offset for -F is in bytes (8k/4 = 2K = $800)
a3090.rom:  a3090 a3090_load
	tools/romload -x -T 000000 -o $@ -F a3090.ld_strip 0x800 a3090_load
	
$(PROGRAM).ld:	$(OBJECTS)
	sc LINK $(OBJECTS) PROGRAMNAME=$@ MAPFILE=$(PROGRAM).map NOICONS

$(PROGRAM).ld_strip: $(PROGRAM).ld
	slink FROM $*.ld TO $@ NODEBUG NOICONS

script.c	   : script.ss
	echo "please rebuild ncr script.c from script.ss"

$(OBJ)ncr.o	   : ncr.c ncr710.h device.h modifiers.h scsi.h ncr_ints.h \
		     scsitask.h iotask.h script.c
$(OBJ)xpt.o	   : xpt.c modifiers.h
$(OBJ)init.o       : init.asm modifiers.i device.i scsidisk_rev.i \
		     ncr710.i scsitask.h iotask.h
$(OBJ)lib.o        : lib.asm modifiers.i board.i ncr710.i scsitask.i iotask.i \
		     device.i
$(OBJ)iotask.o     : iotask.asm modifiers.i iotask.i device.i board.i \
		     ncr710.i scsitask.i
$(OBJ)scsitask.o   : scsitask.asm modifiers.i scsitask.i device.i board.i \
		     ncr710.i iotask.i
$(OBJ)xttask.o     : xttask.asm modifiers.i scsitask.i device.i board.i
$(OBJ)attask.o     : attask.asm modifiers.i scsitask.i device.i board.i
$(OBJ)mountstuff.o : mountstuff.asm modifiers.i device.i
$(OBJ)loadseg.o    : loadseg.asm modifiers.i
$(OBJ)endcode.o    : endcode.asm modifiers.i

version:
	bumprev 40 scsidisk_rev
#	bumprev 40 $(PROGRAM)_rev

clean:
	delete \#?_obj/\#?.o
	delete \#?.ld \#?.ld_strip \#?.info
	delete \#?.map
	delete a3090.rom

backup:
	copy "" vax:scsidisk nopro
	copy notes vax:scsidisk/notes nopro
	copy test vax:scsidisk/test nopro

localbackup:
	copy "" tmp:scsidisk nopro
	copy notes tmp:scsidisk/notes nopro
	copy test tmp:scsidisk/test nopro

# very crude...  make all, copy over
# make sure all is checked in...
internal:
	echo "please make release_xxx, where xxx is v39, cdtv, all or other"

release_all:
	echo "Better be sure everything is checked in!"
	$(MAKE)
	copy \#?.ld\#? \#?_rev\#? `type RCS_Link`/
	copy \#?.ld\#? \#?_rev\#? v39:src/kickstart/scsidisk
	protect v39:src/kickstart/scsidisk/RCS/\#? +wd
	copy `type RCS_Link` v39:src/kickstart/scsidisk/RCS
	copy \#?.ld\#? \#?_rev\#? cdtv:src/ide
	protect cdtv:src/ide/RCS/\#? +wd
	copy `type RCS_Link` cdtv:src/ide/RCS
	echo "done!"
	say "done"
#	copy \#?.ld\#? \#?_rev\#? v37:src/kickstart/scsidisk
#	protect v37:src/kickstart/scsidisk/RCS/\#? +wd
#	copy `type RCS_Link` v37:src/kickstart/scsidisk/RCS

#release_v37:
#	$(MAKE)
#	copy \#?.ld\#? \#?_rev\#? `type RCS_Link`/
#	copy \#?.ld\#? \#?_rev\#? v37:src/kickstart/scsidisk
#	protect v37:src/kickstart/scsidisk/RCS/\#? +wd
#	copy `type RCS_Link` v37:src/kickstart/scsidisk/RCS
#	echo "Better be sure everything is checked in!"

release_v39:
	$(MAKE)
	copy \#?.ld\#? \#?_rev\#? `type RCS_Link`/
	copy \#?.ld\#? \#?_rev\#? v39:src/kickstart/scsidisk
	protect v39:src/kickstart/scsidisk/RCS/\#? +wd
	copy `type RCS_Link` v39:src/kickstart/scsidisk/RCS
	echo "Better be sure everything is checked in!"

release_cdtv:
	$(MAKE)
	copy \#?.ld\#? \#?_rev\#? `type RCS_Link`/
	copy \#?.ld\#? \#?_rev\#? cdtv:src/ide
	protect cdtv:src/ide/RCS/\#? +wd
	copy `type RCS_Link` cdtv:src/ide/RCS
	echo "Better be sure everything is checked in!"

release_other:
	$(MAKE)
	copy \#?.ld\#? \#?_rev\#? `type RCS_Link`/
	echo "Better be sure everything is checked in!"

