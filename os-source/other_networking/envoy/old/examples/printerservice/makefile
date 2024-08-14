all : lpd.service lpr

lpr.o : lpr.c
    lc -. -v -rr -cfist -O lpr.c

lpr : lpr.o
    blink lpr.o to lpr lib lib:amiga.lib NODEBUG

lpd.o : lpd.c lpdbase.h
    lc -. -b0 -d1 -rr -me -ms -cfist -v -O lpd.c

lpd_svc.o : lpd_svc.asm lpdbase.i
    hx68 -a $*.asm -cqy -iinclude: -olpd_svc.o

endcode.o : endcode.asm
    hx68 -a endcode.asm -cqy -iinclude: -oendcode.o

lpd.service : lpd_svc.o lpd.o endcode.o
    blink lpd_svc.o lpd.o endcode.o to lpd.service PLAIN NODEBUG LIB lib:amiga.lib lib:debug.lib
