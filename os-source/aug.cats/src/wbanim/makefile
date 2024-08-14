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

all: 	wbanim	wbanimstdio test
 
.c.o:
		$(CC) $(CFLAGS) $*.c

.asm.o:
		$(ASM) -a $*.asm $(AFLAGS) -o $*.o


wbanim:	wbanim.o $(ANIMO) 
  blink <WITH <
FROM lib:c.o wbanim.o $(ANIMO) 
LIBRARY $(LIBS) $(MYLIBS)
TO wbanim $(LFLAGS)
<

wbanimstdio:	wbanimstdio.o $(ANIMO) 
  blink <WITH <
FROM lib:c.o wbanimstdio.o $(ANIMO) 
LIBRARY $(LIBS) $(MYLIBS)
TO wbanimstdio $(LFLAGS)
<

test:	test.o $(ANIMO)
  blink <WITH <
FROM lib:c.o test.o $(ANIMO) $(M)loadilbm.o $(M)getbitmap.o
LIBRARY $(LIBS) $(MYLIBS)
TO test $(LFLAGS)
<
