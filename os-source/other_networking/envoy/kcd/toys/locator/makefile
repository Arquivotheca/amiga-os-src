all : locator

locator : locator.o locator_tm.o
	blink lib:c.o locator.o locator_tm.o to locator lib lib:lc.lib lib:lcr.lib lib:nipc.lib lib:amiga.lib lib:debug.lib

locator.o : locator.c
	lc -v -O locator.c

locator_tm.o : locator_tm.c
	lc -v -O locator_tm.c

%.c:
