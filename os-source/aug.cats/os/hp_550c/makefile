CC =  SC
ASM = hx68
LFLAGSD = SMALLCODE SMALLDATA BATCH NOICONS VERBOSE
LFLAGS  = $(LFLAGSD) NODEBUG
CFLAGS  = DATA=FAR NMINC NOSTKCHK VERBOSE STREQU ERRORREXX OPTTIME OPTIMIZE DEFINE=AMIGA DEFINE=amiga uchar

ASMFLAGS = incdir include:
LINK = SLINK
LIB = lib:amiga.lib+lib:sc.lib+lib:debug.lib
#OBJ = printertag.o+init.o+data.o+dospecial.o+render.o+transfer.o+ctransfer.o+density.o
OBJ = printertag.o+init.o+data.o+dospecial.o+render.o+transfer.o+density.o
TARGET = HP_550C


.c.o:
   @$(CC) $(CFLAGS) $*.c
   
$(TARGET): printertag.o init.o data.o dospecial.o render.o transfer.o density.o
   @$(LINK) <WITH <
   FROM $(OBJ)
   TO $(TARGET)
   LIBRARY $(LIB)
   $(LFLAGS) MAP $(TARGET).map H
   <

init.o: init.asm
   @$(ASM) from init.asm to init.o incdir include:

printertag.o: printertag.asm $(TARGET)_rev.i 
   @$(ASM) from printertag.asm to printertag.o $(ASMFLAGS)

transfer.o: transfer.asm
   @$(ASM) transfer.asm to transfer.o $(ASMFLAGS)

ctransfer.o: ctransfer.asm
   @$(ASM) ctransfer.asm to ctransfer.o $(ASMFLAGS)


dospecial.o: dospecial.c

data.o: data.c

density.o: density.c

render.o: render.c
