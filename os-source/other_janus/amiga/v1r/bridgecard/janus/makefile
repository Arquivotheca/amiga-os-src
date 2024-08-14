.SUFFIXES: .library .obj .asm

janus.library: \
abs.obj alloc.obj ints.obj init.obj lib.obj defs.obj amigarw.obj endcode.obj \
janus.lib
	alink with lib.wth
#	cp janus.library /usr/amiga

janus.lib: linkage.asm lvos.asm
	assem -i /usr/commodore/amiga/V33/internal/include/ linkage.asm -o linkage.obj
	assem -i /usr/commodore/amiga/V33/internal/include/ lvos.asm -o lvos.obj
	cat lvos.obj linkage.obj > janus.lib
	rm linkage.obj lvos.obj

alloc.obj amigarw.obj config.obj init.obj ints.obj process.obj rconfig.obj: \
janusreg.i janusvar.i asmsupp.i assembly.i janus.i

lib.obj: asmsupp.i assembly.i janus.i

process.obj: i86block.i 

defs.obj: janus.i

abs.obj: assembly.i

init.obj linkage.obj lvos.obj: jfuncs.i

amigarw.obj init.obj process.obj: services.i 

amigarw.obj: memrw.i

alloc.obj: setupsig.i 

.asm.obj:
	assem $*.asm -i /usr/commodore/amiga/V33/internal/include/ -o $*.obj
