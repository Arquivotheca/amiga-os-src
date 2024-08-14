
NETINC  = NETINC:
OBJ    = test.o
 
.asm.o:
        casm -a $*.asm -cqvy -o $*.o

.c.o: 
	lc -v -cst -b1 -O $(CFLAGS) -i$(NETINC) -o$*.o $*.c

all:	test


test: $(OBJ)
	blink from $(OBJ) to test SC SD ND

