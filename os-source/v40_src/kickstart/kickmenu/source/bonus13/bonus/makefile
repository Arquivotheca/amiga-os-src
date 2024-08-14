#
#   Bonus module for V1.3 Kickstart.  Adds all A3000 specific needs to
#   a stock V1.3 Kickstart.
#

.asm.o:
	c $*

.c.o:
	lc -L -v -iINCLUDE: -cs $*


bonus13: savemem.o scsidisk/scsidisk.ld rom_addmem/rom_addmem.ld mmu/mmu13.ld battclock/battclock13.ld buserror/buserror.ld
       -delete bonus13
	clearf
	bumprev 1 bonus13
	cl version
	loadf mmu/mmu13.ld		  ;must be at the start
	loadf rom_addmem/rom_addmem.ld
	loadf scsidisk/scsidisk.ld
	loadf battclock/battclock13.ld
	loadf buserror/buserror.ld
	loadf version			;sorry, mmu must be at the start
	savemem
	killf



clean:
	delete \#?.o
	delete \#?.lnk
       -delete >nil: bonus13


