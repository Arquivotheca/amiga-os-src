.SUFFIXES: .ld .obj .asm

setprefs.ld:  setprefs.obj mainprog.obj gadgets.obj
	alink with setprefs.wth
	cp setprefs.ld /usr2/amiga/setprefs.ld
	rcp setprefs.ld otto:/usr/amiga/setprefs.ld

setprefs.obj:  mydata.i setprefs.asm

mainprog.obj:  mydata.i mainprog.asm

loadprefs.ld:
	alink with loadprefs.wth

.asm.obj:
	assem $*.asm -i /usr/commodore/amiga/V33/internal/include -o $*.obj

