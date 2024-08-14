# This example requires V39 NewIFF modules and include files

CC	= lc
CFLAGS	= -b1 -cfistq -v


ASM	= hx68
AFLAGS	= -cqry -i INCLUDE:

LN	= blink
LFLAGS  =

LIBS    = LIB:lc.lib LIB:amiga.lib
MYLIBS  =

M       = LIB:iffobj/


IFFO  = $(M)Hook.o $(M)ilbmr.o $(M)parse.o $(M)iffpstrings.o
ANIMO = $(IFFO) animr.o unvscomp.o unxorcomp.o unpacker.o

all: 	showanim
 
.c.o:
		$(CC) $(CFLAGS) $*.c

.asm.o:
		$(ASM) -a $*.asm $(AFLAGS) -o $*.o


showanim:	showanim.o $(ANIMO)
  blink <WITH <
FROM lib:c.o showanim.o $(ANIMO) $(M)loadilbm.o $(M)getbitmap.o $(M)screen.o
LIBRARY $(LIBS) $(MYLIBS)
TO showanim $(LFLAGS)
<
