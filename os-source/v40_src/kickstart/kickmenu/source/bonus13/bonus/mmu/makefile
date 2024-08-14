
.asm.o:
	c $*

mmu13.ld: mmu13.o
	blink.old mmu13.o to mmu13.ld LIB lib:amiga.lib NODEBUG

clean:
       -delete \#?.o
       -delete mmu13
