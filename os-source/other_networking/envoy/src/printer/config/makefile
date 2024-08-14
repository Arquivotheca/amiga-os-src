SCSRC = serverconfig.c
SCOBJS = serverconfig.o
CCSRC = clientconfig.c
CCOBJS = clientconfig.o

all : serverconfig clientconfig

serverconfig.o : serverconfig.c
clientconfig.o : clientconfig.c

$(SCOBJS) :
        lc -. -b0 -d1 -ccist -v -O $*.c
#        omd $*.o $*.c >$*.d

$(CCOBJS) :
        lc -. -b0 -d1 -ccist -v -O $*.c
#        omd $*.o $*.c >$*.d


serverconfig : serverconfig.o
            blink lib:astartup.obj+$(SCOBJS) to serverconfig lib lib:lc.lib lib:amiga.lib lib:debug.lib

clientconfig: clientconfig.o
            blink lib:astartup.obj+$(CCOBJS) to clientconfig lib lib:lc.lib lib:amiga.lib lib:debug.lib

bumpc :
        co -l printerclientconfig.rev
        bumprev 37 printerclientconfig
        ci -u printerclientconfig.rev

bumps :
        co -l printerserverconfig.rev
        bumprev 37 printerserverconfig
        ci -u printerserverconfig.rev

release :
        copy clientconfig.c clientconfig serverconfig.c serverconfig makefile envoy:src/printer/config
        copy printerclientconfig#? envoy:src/printer/config
        copy printerserverconfig#? envoy:src/printer/config
        protect envoy:src/printer/config/#? +rewd QUIET



