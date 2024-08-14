# Makefile for A2091/A590 SCSI/XT device driver

.SUFFIXES: .asm .o

SOURCES = init.asm lib.asm iotask.asm scsitask.asm xttask.asm attask.asm \
	  mountstuff.asm loadseg.asm endcode.asm
OBJECTS = $(OBJ)init.o $(OBJ)lib.o $(OBJ)mountstuff.o $(OBJ)iotask.o \
	  $(OBJ)scsitask.o $(OBJ)xttask.o $(OBJ)attask.o $(OBJ)loadseg.o \
          $(OBJ)endcode.o
INCLUDES= device.i board.i hardblocks.i scsidirect.i filehandler.i \
	  filesysres.i

.asm.o:
	casm -A $*.asm -o$@ -iinclude: -c=QS -h$(PROGRAM).i

all:	scsidisk ide a590 a2091

scsidisk:
	lmk scsidisk.ld.strip OBJ=scsidisk_obj/ PROGRAM=scsidisk

ide:
	lmk ide.ld.strip OBJ=ide_obj/ PROGRAM=ide

a2091:
	lmk a2091.ld.strip OBJ=a2091_obj/ PROGRAM=a2091

a590:
	lmk a590.ld.strip OBJ=a590_obj/ PROGRAM=a590

a2090:
	lmk a2090.ld.strip OBJ=a2090_obj/ PROGRAM=a2090

scsidisk.ld:	$(OBJECTS)
	blink <WITH <
FROM	$(OBJECTS)
TO	$(PROGRAM).ld
LIBRARY	lib:debug.lib debug.lib lib:amiga.lib
<

scsidisk.ld.strip: scsidisk.ld
	blink scsidisk.ld to scsidisk.ld.strip ND

ide.ld:		$(OBJECTS)
	blink <WITH <
FROM	$(OBJECTS)
TO	$(PROGRAM).ld
LIBRARY	lib:debug.lib debug.lib lib:amiga.lib
MAP $(PROGRAM).map HLSFX PLAIN
<

ide.ld.strip: ide.ld
	blink ide.ld to ide.ld.strip ND

a590.ld:	$(OBJECTS)
	blink <WITH <
FROM	$(OBJECTS)
TO	$(PROGRAM).ld
LIBRARY	lib:debug.lib debug.lib lib:amiga.lib
<

a590.ld.strip: a590.ld
	blink a590.ld to a590.ld.strip ND

a2091.ld:	$(OBJECTS)
	blink <WITH <
FROM	$(OBJECTS)
TO	$(PROGRAM).ld
LIBRARY	lib:debug.lib debug.lib lib:amiga.lib
<

a2091.ld.strip: a2091.ld
	blink a2091.ld to a2091.ld.strip ND

a2090.ld:	$(OBJECTS)
	blink <WITH <
FROM	$(OBJECTS)
TO	$(PROGRAM).ld
LIBRARY	lib:debug.lib debug.lib lib:amiga.lib
<

a2090.ld.strip: a2090.ld
	blink a2090.ld to a2090.ld.strip ND

$(OBJ)init.o       : init.asm modifiers.i device.i scsidirect.i scsidisk_rev.i
$(OBJ)lib.o        : lib.asm modifiers.i board.i hardblocks.i scsidirect.i
$(OBJ)iotask.o     : iotask.asm modifiers.i iotask.i device.i board.i scsidirect.i
$(OBJ)scsitask.o   : scsitask.asm modifiers.i scsitask.i device.i board.i scsidirect.i
$(OBJ)xttask.o     : xttask.asm modifiers.i scsitask.i device.i board.i scsidirect.i
$(OBJ)attask.o     : attask.asm modifiers.i scsitask.i device.i board.i scsidirect.i
$(OBJ)mountstuff.o : mountstuff.asm modifiers.i device.i hardblocks.i filesysres.i filehandler.i
$(OBJ)loadseg.o    : loadseg.asm modifiers.i
$(OBJ)endcode.o    : endcode.asm modifiers.i

version:
	bumprev 37 scsidisk_rev
#	bumprev 37 $(PROGRAM)_rev

clean:	scsidisk_clean ide_clean a590_clean a2091_clean a2090_clean

scsidisk_clean:
	-delete scsidisk_obj/\#?.o
	-delete scsidisk.ld scsidisk.ld.strip
ide_clean:
	-delete ide_obj/\#?.o
	-delete ide.ld ide.ld.strip

a590_clean:
	-delete a590_obj/\#?.o
	-delete a590.ld a590.ld.strip

a2091_clean:
	-delete a2091_obj/\#?.o
	-delete a2091.ld a2091.ld.strip

a2090_clean:
	-delete a2091_obj/\#?.o
	-delete a2091.ld a2091.ld.strip

backup:
	copy "" vax:scsidisk nopro

localbackup:
	copy "" tmp:scsidisk nopro

