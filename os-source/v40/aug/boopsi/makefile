# boopsi Intuition demos
# LATTICE
# created March 6, 1990

##### setup	#####
SYM=demo.sym
SYMSRC= sysall.h
SYMDIR=ram:
BACKUPDIR=A:backup/amiga/newexamples/boopsi
INSTALLDIR=V:aug/boopsi
#SHUTTLE=copy to df0: from
SHUTTLE=@list

# INCLUDE assign volumes used (or change this line)
# VINCLUDE:	v36 include (stripped) directory
# INCLATTICE:	Lattice's standard include directory, e.g. <stdio.h>
# MYINCLUDE:	development overrides of system includes,
#		 you can remove this, or use it for interim bug
#		 fixes in the include files
# VLIB:		location of V36 amiga.lib, debug.lib
INCLUDE= -iMYINCLUDE: -iVINCLUDE: -iINCLATTICE:
AINCLUDE= 

# turn this thing on if you turn on any debugging that
# calls dumpTagItem()
#TAGDEBUG=tagdebug.o
TAGDEBUG=

##### options	#####
# lc pass 1
CFLAGS1=-. $(INCLUDE) -d -cwu -H$(SYMDIR)$(SYM)
# lc pass 2
CFLAGS2=-. -v
# linker
BLOPTS=BATCH BUFSIZE 1024 QUIET SC SD
LIBS=LIB:lc.lib VLIB:debug.lib VLIB:amiga.lib 

##### rules	#####
# lattice compiler, separate passes for no particular reason
.c.o:
	@echo "	$*.c pass 1"
	@lc1 $(CFLAGS1) -oquad:$*.q $* 
	@echo "	optimization pass"
	@go quad:$*.q
	@echo "	$*.c pass 2"
	@lc2 $(CFLAGS2) -o$*.o quad:$*.q 

ASM=casm -a
AFLAG= -cvrf
#AINCL= -i INCLUDE:
AINCL= -i MYINCLUDE: -i VINCLUDE:
.asm.o:
	@echo "$*.asm: $(AFLAGS)"
	@$(ASM) $*.asm $(AFLAGS) $(AINCL) -o $*.o

# produce a "listing" file (by disassembly)
.o.lst:
	omd >$*.lst $*.o $*.c

###### targets #####
all: $(SYMDIR)$(SYM) demoframe demotextb pubi demopubi demoimage \
	demo1 demo2 demo3 demo4 demo5 myclass.ld democlasslib

# simple private image class example
DEMOIO=demoimage.o emboxclass.o classface.o hookface.o
demoimage: $(DEMOIO)
	blink $(BLOPTS) FROM lib:c.o $(DEMOIO) LIB $(LIBS) TO $@
	$(SHUTTLE) $@

# public image class example
DEMOPUBO=demopubi.o emboxpubcl.o classface.o hookface.o
demopubi: $(DEMOPUBO)
	blink $(BLOPTS) FROM lib:c.o $(DEMOPUBO) LIB $(LIBS) TO $@
	$(SHUTTLE) $@

# public image class installer: program version
PUBIO=pubi.o emboxpubcl.o classface.o hookface.o 
pubi: $(PUBIO)
	blink $(BLOPTS) FROM lib:c.o $(PUBIO) LIB $(LIBS) TO $@
	$(SHUTTLE) $@

# public image class installer: library version
MYCLASSO=myclasslib.o myclassinit.o emboxpubcl.o classface.o hookface.o 
myclass.ld:  $(MYCLASSO)
	blink $(BLOPTS) FROM $(MYCLASSO) \
	LIB  VLIB:debug.lib VLIB:amiga.lib  \
	TO $@
	stripa $@ to myclass.library

#	copy myclass.library to df0:libs

DEMOCLASSLIBO=democlasslib.o
democlasslib: $(DEMOCLASSLIBO)
	blink $(BLOPTS) FROM lib:c.o $(DEMOCLASSLIBO) LIB $(LIBS) TO $@
	$(SHUTTLE) $@

# public image "frame" class example
DEMOFRAMEO=demoframe.o frame1class.o classface.o hookface.o
demoframe: $(DEMOFRAMEO)
	blink $(BLOPTS) FROM lib:c.o $(DEMOFRAMEO) LIB $(LIBS) TO $@
	$(SHUTTLE) $@

# text button class, using frame image around text contents
DEMOTBO= demotextb.o textbclass.o frame1class.o \
	classface.o hookface.o
demotextb: $(DEMOTBO)
	blink $(BLOPTS) FROM lib:c.o $(DEMOTBO) LIB $(LIBS) TO $@
	$(SHUTTLE) $@

##### gadget examples	#####

# boopsi gadgets and GADGETUP messages
DEMO1O=demo1.o classface.o
demo1: $(DEMO1O)
	blink $(BLOPTS) FROM lib:c.o $(DEMO1O) LIB $(LIBS) TO $@
	$(SHUTTLE) $@

# boopsi gadgets and IDCMPUPDATE messages
DEMO2O=demo2.o classface.o
demo2: $(DEMO2O)
	blink $(BLOPTS) FROM lib:c.o $(DEMO2O) LIB $(LIBS) TO $@
	$(SHUTTLE) $@

# boopsi gadgets, a model, ic's, and boopsi interconnections
DEMO3O=demo3.o mymodelclass.o hookface.o classface.o $(TAGDEBUG)
demo3: $(DEMO3O)
	blink $(BLOPTS) FROM lib:c.o $(DEMO3O) LIB $(LIBS) TO $@
	$(SHUTTLE) $@

# an interconnected boopsi gadget group
DEMO4O=demo4.o mymodelclass.o hookface.o classface.o $(TAGDEBUG)
demo4: $(DEMO4O)
	blink $(BLOPTS) FROM lib:c.o $(DEMO4O) LIB $(LIBS) TO $@
	$(SHUTTLE) $@

# a class that creates boopsi composite gadgets
DEMO5O=demo5.o mygroupgclass.o mymodelclass.o hookface.o classface.o $(TAGDEBUG)
demo5: $(DEMO5O)
	blink $(BLOPTS) FROM lib:c.o $(DEMO5O) LIB $(LIBS) TO $@
	$(SHUTTLE) $@


##### 	#####

backup:
	copy \#? $(BACKUPDIR)

install:
	copy \#? $(INSTALLDIR)

##### make precompiled header files	#####

$(SYMDIR)$(SYM): $(SYM)
	copy $(SYM) $(SYMDIR)

$(SYM): $(SYMSRC)
	lc1 $(INCLUDE) -ph -o$(SYM) $(SYMSRC)

