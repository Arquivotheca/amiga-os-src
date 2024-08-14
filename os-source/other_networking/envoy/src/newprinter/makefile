COBJS = lpd.o
AOBJS = lpd_svc.o endcode.o
SVC_OBJS = lpd_svc.o lpd.o endcode.o

all : lpd.service

lpd.o : lpd.c lpdbase.h
lpd_svc.o : lpd_svc.asm lpdbase.i
endcode.o : endcode.asm

$(COBJS) :
    lc -. -b0 -d1 -rr -me -ms -cfist -v -O $*.c
#    omd $*.o $*.c >$*.d

%.asm:

%.c:

$(AOBJS) :
    casm -a $*.asm -cqy -iinc: -o$*.o

lpd.service : $(SVC_OBJS)
    blink $(SVC_OBJS) to lpd.service PLAIN NODEBUG LIB lib:small.lib lib:debug.lib
