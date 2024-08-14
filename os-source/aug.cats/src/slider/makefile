
#MYLIBS= LIB:debug.lib

CC = SC
ASM = asm

CFLAGS  = data=near nominc streq nostkchk
AFLAGS  = -iINCLUDE:
LINKER  = slink
LFLAGS  = SMALLCODE BATCH NODEBUG
STARTUP = LIB:c.o
LIBS    = LIB:sc.lib, LIB:amiga.lib

M      = iffobj/
A      = 


# Our iffparse support object modules to link with
IFFO      = $(M)parse.o $(M)iffpstrings.o $(M)Hook.o
ILBMRO    = $(M)ilbmr.o $(M)unpacker.o
ILBMSO    = $(M)getdisplay.o $(M)screen.o
ILBMLO    = $(M)loadilbm.o $(M)getbitmap.o
ILBMRSO   = $(IFFO) $(ILBMRO) $(ILBMSO)
ILBMWO    = $(M)saveilbm.o $(M)ilbmw.o $(M)packer.o
ILBMO     = $(IFFO) $(ILBMRO) $(ILBMLO) $(ILBMSO) $(ILBMWO)
EXTRAO    = $(M)copychunks.o $(M)screendump.o $(M)bmprintc.o


# Our iffparse applications
APP1   = $(A)slider

# The object modules needed by each application example
APP1O  = $(APP1).o $(ILBMO)

.SUFFIXES:
.SUFFIXES:	.o .c .h .asm .i

# Make all of the applications
all:	$(APP1)

# Linkage for each application

$(APP1): $(APP1O)
  $(LINKER) <WITH <
FROM $(STARTUP) $(APP1O)
LIBRARY $(LIBS) $(MYLIBS)
TO $(APP1) $(LFLAGS)
<

.c.o:
	$(CC) $(CFLAGS) $*.c

.asm.o:
	$(ASM) $(AFLAGS) $*.asm

