# Makefile for A2091/A590 SCSI/XT device driver

.SUFFIXES: .asm .o

PROGRAM = diskdev

SOURCES = init.asm lib.asm iotask.asm scsitask.asm xttask.asm mountstuff.asm\
          loadseg.asm endcode.asm
OBJECTS = init.o lib.o iotask.o scsitask.o xttask.o mountstuff.o\
          loadseg.o endcode.o
INCLUDES= device.i board.i hardblocks.i scsidirect.i filehandler.i filesysres.i

.asm.o:
	casm -A $*.asm -o$@ -i:include -c=Q

$(PROGRAM):  $(OBJECTS)
	blink with $(PROGRAM).wth
	copy $(PROGRAM) :expansion/stevedisk.device

init.o       : modifiers.i device.i scsidirect.i
lib.o        : modifiers.i board.i hardblocks.i scsidirect.i
iotask.o     : modifiers.i iotask.i device.i board.i scsidirect.i
scsitask.o   : modifiers.i scsitask.i device.i board.i scsidirect.i
xttask.o     : modifiers.i scsitask.i device.i board.i scsidirect.i
mountstuff.o : modifiers.i device.i hardblocks.i filesysres.i filehandler.i
loadseg.o    : modifiers.i
endcode.o    : modifiers.i
