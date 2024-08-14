
#MYLIBS= LIB:debug.lib

CC = sc
ASM = HX68

CFLAGS  = data=near nominc streq nostkchk errorrexx
AFLAGS  = -i INCLUDE:
LINKER  = slink
LFLAGS  = SMALLCODE BATCH NODEBUG NOICONS
STARTUP = LIB:c.o
LIBS    = LIB:sc.lib, LIB:amiga.lib LIB:debug.lib

M      =
A      =


# Our iffparse support object modules to link with
IFFO      = $(M)parse.o $(M)iffpstrings.o $(M)Hook.o
ILBMWO    = $(M)saveilbm.o $(M)ilbmw.o $(M)packer.o
OTHERO	  = lvwedge.o

# Our iffparse applications
APP1   = $(A)Grabber

# The object modules and special includes needed by each application example
APP1O  = $(APP1).o $(IFFO) $(ILBMWO) $(OTHERO)

.SUFFIXES:
.SUFFIXES:	.o .c .h .asm .i

# Make all of the applications
all: $(APP1)

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
	 $(ASM) -a $*.asm $(AFLAGS) -o $*.o
 
