CSRC = fstool.c
COBJS = fstool.o

all : fstool

fstool.o : fstool.c

$(COBJS) :
        lc -. -b0 -d5 -ccist -v -O $*.c

fstool : $(COBJS)
         blink lib:astartup.obj+$(COBJS) to fstool lib lib:lc.lib lib:amiga.lib lib:debug.lib ND

bump :
        bumprev 37 fstool

