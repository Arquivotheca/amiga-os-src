#
#   Kickstart 1.4 bonus module
#

.asm.o:
	c $*

.c.o:
	lc -L -v -iINCLUDE: -cs $*

bonus:	mmu20.o savemem.o
	bumprev 37 bonus20
	cl version
	blink mmu20.o to mmu20.ld LIB lib:amiga.lib NODEBUG
	;clearf
	;loadf mmu20.ld
	;savemem

clean:
       -delete \#?.o
       -delete \#?.lnk
       -delete >nil: mmu20
