# dropshadow -- copyright james mackraz, 1987

LFLAGS=-ltool -lc -w

PREDIR=VDK:
OBJDIR=RAM:

PRE=ds.pre
SRC=ds.c myhooks.c prop.c
ASRC=hooks.asm
OBJ=ds.o myhooks.o hooks.o prop.o
INCL=ds.h sysall.h
PRESRC=preds.c

ARCV=$(SRC) $(ASRC) $(INCL) $(PRESRC) makefile readme ds

CFLAGS=+I$(PREDIR)$(PRE)

.c.o:
	cc $(CFLAGS) -o $*.o $*.c

#all: ds
all: $(PREDIR)$(PRE) dice

ds.arc:
	arc a ds.arc $(ARCV)
	arc t ds.arc

dice: $(OBJ) makefile
	ln $(OBJ) $(LFLAGS) -o $@
#	copy ds usr:bin

$(OBJ): $(INCL)

$(PREDIR)$(PRE): $(PRE)
	copy $(PRE) $(PREDIR)

$(PRE): sysall.h
	cc -A -o $(OBJDIR)pre_garbage.asm +H$(PRE) $(PRESRC)

tags: $(SRC) makefile
	ctags $(SRC)

