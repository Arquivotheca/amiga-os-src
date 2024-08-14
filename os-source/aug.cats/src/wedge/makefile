
HDRS = 

#MYLIBS= LIB:debug.lib LIB:ddebug.lib
TARGET = wedge
LOCAL  = h:srcc/1.4/wedge
REMOTE = V36:src/workbench/utilities/wedge

CC = sc
ASM = HX68

CFLAGS  = data=far nominc streq nostkchk errorrexx
AFLAGS  = -i INCLUDE:

LINKER  = slink
LFLAGS  = SMALLCODE BATCH NODEBUG
STARTUP = LIB:astartup.obj
LIBS    = LIB:amiga.lib, LIB:sc.lib

# NOTE - we are using local copies of dofmt,ktty,dtty,rawio so that
# our kprintfs/dprintfs need not do through locale's wedge in RawDoFmt 
# Modified rawio.asm (ddebug) and ktty (debug) to not use exec's RawDoFmt
# but still use exec's RawPutChar
OFILES =	$(TARGET).o $(TARGET)a.o ktty.o dtty.o rawio.o dofmt.o
CFILES	=	$(TARGET).c
AFILES  =	$(TARGET)a.asm ktty.asm dtty.asm rawio.asm dofmt.asm


.SUFFIXES:
.SUFFIXES:	.o .c .h .asm .i

$(TARGET).ld:	$(OFILES)
	$(LINKER) FROM $(STARTUP) $(OFILES) LIB $(LIBS) $(MYLIBS) \
	TO $(TARGET) ${LFLAGS}

.c.o:
	$(CC) $(CFLAGS) $*.c
.asm.o:
        $(ASM) -a $*.asm $(AFLAGS) -o $*.o
	
$(TARGET).o:   $(TARGET).c
$(TARGET)a.o: $(TARGET)a.asm

version:
	@bumprev 36 $(TARGET)_rev

local:
	@copy \#?.c|\#?.asm|\#?.h|\#?.i|Makefile|\#?.ld|lmkfile|\#?.rev $(LOCAL)

internal:
	@copy \#?.c|\#?.asm|\#?.h|\#?.i|Makefile|\#?.ld|lmkfile|\#?.rev $(REMOTE)

clean:
	@delete \#?.o|\#?.ld

