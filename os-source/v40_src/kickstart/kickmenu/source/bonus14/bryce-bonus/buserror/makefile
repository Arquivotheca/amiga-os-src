
.asm.o:
	c $*

buserror.ld:   buserror.o debug.o
	blink.old buserror.o+debug.o to buserror.ld LIB lib:amiga.lib NODEBUG

clean:
	delete \#?.ld
	delete \#?.o
