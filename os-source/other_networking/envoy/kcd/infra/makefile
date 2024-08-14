all: InfraNet RemoteControl

%.o : %.c
	lc -o$@ -O -v $<

InfraNet : InfraNet.o
	blink from lib:c.o infranet.o to InfraNet lib lib:lc.lib lib:amiga.lib lib:nipc.lib lib:debug.lib

RemoteControl : RemoteControl.o
	blink from lib:c.o RemoteControl.o to RemoteControl lib lib:lc.lib lib:amiga.lib lib:nipc.lib lib:debug.lib

%.c:

Infranet.o : InfraNet.c
RemoteControl.o : RemoteControl.c