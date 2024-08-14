CFLAGS= -cufist -d
OFILES= main.o gfmath.o syndec.o poly.o  gfsupport.o
MATFILES = matbuild.o gfmath.o gfsupport.o
.c.o:
	LC $(CFLAGS) $*.c
.asm.o:
        asm $(AFLAGS) $*.asm

ecc:	$(OFILES) makefile
	blink FROM LIB:c.o $(OFILES) TO ecc \
	LIBRARY LIB:lc.lib LIB:amiga.lib VERBOSE ADDSYM

matbuild: $(MATFILES) makefile
	blink FROM LIB:c.o $(MATFILES) TO matbuild \
	LIBRARY LIB:lc.lib LIB:amiga.lib VERBOSE ADDSYM


gfmath.o:	gfmath.c gf64.def
proto:
	-delete	ram:\#?.i
	lc -pr gfmath matsupport syndec
	join	ram:\#?.i AS ecc_protos.h

