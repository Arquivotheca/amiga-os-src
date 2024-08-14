ASRC = a2060.asm config.asm int.asm write.asm read.asm general.asm timer.asm
AOBJS = a2060.o config.o int.o write.o read.o general.o timer.o

all : a2060.device

$(AOBJS) :
           casm -a $*.asm -o $*.o -i include: -cry

a2060.o    : a2060.asm a2060.i
config.o   : config.asm a2060.i
int.o      : int.asm a2060.i
write.o    : write.asm a2060.i
read.o     : read.asm a2060.i
general.o  : general.asm a2060.i

a2060.device : $(AOBJS)
               blink $(AOBJS) to a2060.device lib lib:amiga.lib lib:debug.lib NODEBUG\
               MAP a2060.map S X F H
               copy a2060.device devs:

opentest :     opentest.c
               lc -v -b0 opentest.c
               blink lib:c.o+opentest.o to opentest lib lib:lc.lib lib:amiga.lib lib:debug.lib NODEBUG


