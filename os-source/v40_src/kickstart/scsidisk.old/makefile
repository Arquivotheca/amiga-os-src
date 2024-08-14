# Makefile for A2091/A590 SCSI/XT device driver

.SUFFIXES: .asm .o

PROGRAM = scsidisk

SOURCES = init.asm lib.asm iotask.asm scsitask.asm xttask.asm mountstuff.asm\
          loadseg.asm endcode.asm
OBJECTS = init.o lib.o iotask.o scsitask.o xttask.o mountstuff.o\
          loadseg.o endcode.o
INCLUDES= device.i board.i hardblocks.i scsidirect.i filehandler.i filesysres.i

.asm.o:
	casm -A $*.asm -o$@ -iinclude: -c=QS

$(PROGRAM):  $(OBJECTS)
	blink with $(PROGRAM).wth
	blink $(PROGRAM).ld to $(PROGRAM).ld.strip ND

init.o       : modifiers.i device.i scsidirect.i scsidisk_rev.i
lib.o        : modifiers.i board.i hardblocks.i scsidirect.i
iotask.o     : modifiers.i iotask.i device.i board.i scsidirect.i
scsitask.o   : modifiers.i scsitask.i device.i board.i scsidirect.i
xttask.o     : modifiers.i scsitask.i device.i board.i scsidirect.i
mountstuff.o : modifiers.i device.i hardblocks.i filesysres.i filehandler.i
loadseg.o    : modifiers.i
endcode.o    : modifiers.i

version:
	bumprev 37 $(PROGRAM)_rev
