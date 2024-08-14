.SUFFIXES: .ld .obj .asm

handler.ld:  handler.obj
	alink with handler.wth

handler.obj:  handler.asm diagmsg.i

test.ld:   test.obj
	alink with test.wth

test.obj:  test.asm diagmsg.i

.asm.obj:
	assem $*.asm -i /usr/commodore/amiga/V33/internal/include -o $*.obj
