#
#   debug.o
#
#

.asm.o:
	c $*


rom_addmem.ld: rom_addmem.o
	blink.old rom_addmem.o lib lib:amiga.lib to rom_addmem.ld ND

clean:
       -delete \#?.o
       -delete rom_addmem.ld
