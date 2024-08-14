# ScDemo screen demo
# LATTICE
# created aug 17, 1989

##### setup	#####
SYM=itests.sym
SYMSRC= sysall.h
SYMDIR=rad:
LCLIB=rad:lc.lib 
SHUTTLE=df1:
INCLUDE= -iA:v36/include -iINCLUDE:

##### options	#####
# lc pass 1
CFLAGS1=-. $(INCLUDE) -d -cwus -H$(SYMDIR)$(SYM)
# lc pass 2
CFLAGS2=-. -v
# cape assembler
AFLAGS=-iINCLUDE: -cvrf
# linker
BLOPTS=BATCH BUFSIZE 1024 QUIET SC SD
# use this if you are using 'struct custom Custom;'
#BLOPTS=BATCH BUFSIZE 1024 QUIET SC SD DEFINE _custom=14675968
#BLOPTS=BATCH BUFSIZE 1024 QUIET SC SD

##### rules	#####
# lattice compiler
.c.o:
	@echo "	$*.c pass 1"
	lc1 $(CFLAGS1) -oquad:$*.q $* 
	go quad:$*.q
	@echo "	$*.c pass 2"
	lc2 $(CFLAGS2) -o$*.o quad:$*.q 

# cape assembler rule
.asm.o:
	casm -a $*.asm -o$*.o $(AFLAGS)

# produce a "listing" file (by disassembly)
.o.lst:
	omd >$*.lst $*.o $*.c

# produce a library pragma file (.p file) from .fd file
.fd.p:
	fd2pragma $*.fd $*.p

###### targets #####
all: $(SYMDIR)$(SYM) scdemo

SCDEMOO=scdemo.o
scdemo: $(SCDEMOO)
	blink $(BLOPTS) FROM lib:c.o $(SCDEMOO) LIB $(LCLIB) TO $@
	copy $@ to $(SHUTTLE)

scdemo.lst: scdemo.o


##### kprintf stub	#####
# i forgot just how to use this.
kp.omod: kp.o
	blink PRELINK $(BLOPTS) FROM kp.o LIB LIB:debug.lib LIB:amiga.lib TO $@

##### make precompiled header files	#####

$(SYMDIR)$(SYM): $(SYM)
	copy $(SYM) $(SYMDIR)

$(SYM): $(SYMSRC)
	lc1 $(INCLUDE) -ph -o$(SYM) $(SYMSRC)

##### utilities	#####
clean:
	delete \#?.lst \#?.o \#?.q \#?.p
