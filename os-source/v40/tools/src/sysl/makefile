BOPTS = $$JUMPS,$$MACRO,$$EXT,$$WORD,w30000

.SUFFIXES:
.SUFFIXES:  .b  .asm  .o  .obj  .ld

.b.o:
	bcpl $*.b $*.o opt '"$$UNIX4,$$MCC,$(BOPTS)"'
.asm.o:
	./bassem $*.asm to $*.o opt u

.b.obj:; bcpl $*.b $*.obj opt '"%$$AMIDOS,$$AMIGA,$(BOPTS)"' obj A
.asm.obj:; cassem $*.asm -o $*.obj -i $(INC)

sysl:		first.o blib.o sysl.o last.o
	rsh mailhost mkdir qwe.sysl
	rcp first.o blib.o sysl.o last.o mailhost:qwe.sysl
	rsh mailhost "cd qwe.sysl ; cc -O first.o blib.o sysl.o last.o -o sysl"
	rcp mailhost:qwe.sysl/sysl sysl
	rsh mailhost rm -r qwe.sysl

amigahdr:
	cp $(DOS)LIBHDR .

unixhdr:
	cp $(BLIB)LIBHDR .

sysl.o:		sysl.b
blib.o:		blib.b
first.o:	first.asm LIBHDR-i SYSHDR-i
last.o:		last.asm

EXECUTABLES=	genlib.out

.c.out:
	rcp $? mailhost:qwe.$?
	rsh mailhost cc -O qwe.$? -o qwe.$@
	rcp mailhost:qwe.$@ $@
	rsh mailhost rm qwe.$? qwe.$@

all:		${EXECUTABLES}

install:
	for i in ${EXECUTABLES} ; do cp $$i ../../bin/`basename $$i .out`; done;
