DOS   = ../DOS/
COM   = $(DOS)com/
VER   = /usr/commodore/amiga/V/
INC   = $(VER)internal/include
BOPTS = $$JUMPS,$$MACRO,$$EXT,$$WORD,w30000
SUPP  = $(DOS)support/

.SUFFIXES:
.SUFFIXES:  .b  .asm  .o  .obj  .ld

.b.o:;	bcpl $*.b $*.o opt '"$$UNIX4,$$MCC,$(BOPTS)"'
.asm.o:; ./bassem $*.asm to $*.o opt u


.b.obj:; bcpl $*.b $*.obj opt '"%$$AMIDOS,$$AMIGA,$(BOPTS)"' obj A
.asm.obj:; ./assem $*.asm -o $*.obj -i $(INC)

#
# In-house Version for SUN
#
bcpl:		first.o blib.o fpsim.o last.o \
		start.o tidyup.o args.o lex.o syn.o trna.o trnb.o \
		cg1.o cg2.o cg3.o cg4.o cg5.o cg6.o cg7.o cg8.o
	cc first.o blib.o fpsim.o \
	    start.o tidyup.o args.o lex.o syn.o trna.o trnb.o \
	    cg1.o cg2.o cg3.o cg4.o cg5.o cg6.o cg7.o cg8.o \
	    last.o -o bcpl

#
# Amiga Native Version
# N.B we don't need cg8 (TOS and GST formats)
#
bcpl.ld : amigahdr start.obj tidyup.obj args.obj lex.obj syn.obj \
          trna.obj trnb.obj cg1.obj cg2.obj cg3.obj cg4.obj \
          cg5.obj cg6.obj cg7.obj $(SUPP)fpsim.obj
	cat start.obj tidyup.obj args.obj lex.obj syn.obj trna.obj \
	    trnb.obj cg1.obj cg2.obj cg3.obj cg4.obj cg5.obj cg6.obj \
	    cg7.obj $(SUPP)fpsim.obj > bcpl.obj
	alink from $(COM)interface.om,bcpl.obj to bcpl.ld
	rm bcpl.obj
#
# Ensure we get the amiga LIBHDR here
#
amigahdr:
	cp $(DOS)LIBHDR .
#
# A quick test program
#
bcp : bcp.c
	cc bcp.c -o bcp

unixhdr:
	cp $(BLIB)LIBHDR .
hello: tests/ext.b bcpl bcp
	bcp -v tests/ext.b $(BLIB)bcplfpsim.o -o hello 
	hello
# 
# Objects for UNIX version of compiler
#
start.o  : start.b BCPHDR SYNHDR 
tidyup.o : tidyup.b BCPHDR
args.o   : args.b BCPHDR 
lex.o    : lex.b BCPHDR SYNHDR 
syn.o    : syn.b BCPHDR SYNHDR
trna.o   : trna.b BCPHDR TRNHDR
trnb.o   : trnb.b BCPHDR TRNHDR
cg1.o    : cg1.b BCPHDR CGHDR
cg2.o    : cg2.b BCPHDR CGHDR
cg3.o    : cg3.b BCPHDR CGHDR
cg4.o    : cg4.b BCPHDR CGHDR
cg5.o    : cg5.b BCPHDR CGHDR
cg6.o    : cg6.b BCPHDR CGHDR
cg7.o    : cg7.b BCPHDR CGHDR
cg8.o    : cg8.b BCPHDR CGHDR
#
# Objects for Native Amiga Version
#
start.obj  : start.b BCPHDR SYNHDR 
tidyup.obj : tidyup.b BCPHDR
args.obj   : args.b BCPHDR 
lex.obj    : lex.b BCPHDR SYNHDR 
syn.obj    : syn.b BCPHDR SYNHDR
trna.obj   : trna.b BCPHDR TRNHDR
trnb.obj   : trnb.b BCPHDR TRNHDR
cg1.obj    : cg1.b BCPHDR CGHDR
cg2.obj    : cg2.b BCPHDR CGHDR
cg3.obj    : cg3.b BCPHDR CGHDR
cg4.obj    : cg4.b BCPHDR CGHDR
cg5.obj    : cg5.b BCPHDR CGHDR
cg6.obj    : cg6.b BCPHDR CGHDR
cg7.obj    : cg7.b BCPHDR CGHDR
#
# End of Makefile

/tmp/p: ud /tmp/first.t
	ud /tmp/first.t to /tmp/p

ud: ud.o
	bcp ud.o -o ud

t: test.o first.o last.o blib.o
	cc first.o blib.o test.o last.o -o t
	t infile outfile

p: test.o libext.o first.o last.o blib.o
	cc first.o blib.o test.o libext.o last.o -o p

first.o: first.asm LIBHDR-i SYSHDR-i
last.o: last.asm
blib.o: blib.b LIBHDR SYSHDR
test.o: test.b LIBHDR
libext.o: libext.asm LIBHDR-i
ud.o: ud.b
