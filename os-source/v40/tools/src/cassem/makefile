GCC=cc
CC=cc
SUNCC=cc
#CFLAGS= -O -DAMI -DSUN -pg
#CFLAGS= -pg -O -DAMI -DSUN
#CFLAGS= -DASM -DAMI -DSUN -X98 -X122 -S
CFLAGS= -c -DBTST_FIX -DAMI -DSUN -DASM -O
LDFLAGS= -DAMI -DSUN -O

cassem: asm1.o asm2.o asm3.o asm4.o asm5.o \
     asm6.o asm7.o asm8.o asm9.o blib.o 
	$(SUNCC) $(LDFLAGS) blib.o asm1.o asm2.o asm3.o asm4.o \
           asm5.o asm6.o asm7.o asm8.o asm9.o \
           -o cassem

ahdr: asmhdr
	cp asmhdr ahdr
	ed ahdr <edcomms

.c.o:
	$(GCC) $(CFLAGS) $<
#	as -o $*.o -10 $*.s
#	rm $*.s

asm1.o: asm1.c ahdr 
asm2.o: asm2.c asmhdr 
asm3.o: asm3.c asmhdr 
asm4.o: asm4.c asmhdr 
asm6.o: asm6.c asmhdr 
asm6.d: asm6.c asmhdr 
asm7.o: asm7.c asmhdr 
asm7.d: asm7.c asmhdr 
asm8.o: asm8.c asmhdr 
asm9.o: asm9.c asmhdr 
blib.o: blib.c 

