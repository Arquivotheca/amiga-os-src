#
# smakefile for SetAlert
#
VERSION = 40

#
# Name of the module
#
MODNAME	= setalert

#
# The ".LD" file we want to produce.
#
LD_FILE	= $(MODNAME).ld
LD_STRIP= $(MODNAME)

#
# Compiler, assembler, and linker
#
CC = sc
AS = hx68 -a
LN = slink

#
# Compiler flags...
#
CFLAGS	= IGNORE=306 MEMORYSIZE=HUGE DATA=FARONLY PARAMETERS=REGISTER NOSTACKCHECK NOMULTIPLEINCLUDES STRINGMERGE STRUCTUREEQUIVALENCE MULTIPLECHARACTERCONSTANTS DEBUG=LINE NOVERSION OPTIMIZE OPTIMIZERINLINE OPTIMIZERINLOCAL NOICONS

#
# Assembler flags...  NOTE how they are used below...
#
AFLAGS	= -csq -i include: -o

#
# Linker flags
#
LFLAGS		= QUIET ADDSYM SMALLCODE SMALLDATA MAP $(MODNAME).map HLSXF PLAIN SWIDTH 16 NOICONS
LFLAGS_STRIP	= QUIET ND NOICONS

#
# System libraries
#
LIBS= LIB:amiga.lib LIB:debug.lib

#
# My headers...
#
MY_HDR	=

#
# My C source...
#
MY_C	=	setalert.c

#
# My Assebly source...
#
MY_A	=

#
# My object files...
#
MY_OBJS	=	setalert.o \
		setalert_strings.o

#
# My library objects...
#
MY_LIB_OBJS =

#
# My library files...
#
MY_LIBS	=

#
# Build the stripped LD file...
#
$(LD_STRIP):	$(LD_FILE)
	$(LN) FROM $(LD_FILE) TO $(LD_STRIP) $(LFLAGS_STRIP)
	List $(LD_STRIP)

#
# Build the LD file...
#
$(LD_FILE):	$(MY_OBJS) $(MY_LIBS)
	$(LN) <with <
FROM $(MY_OBJS)
TO $(LD_FILE)
LIB $(MY_LIBS) $(LIBS)
$(LFLAGS)
<

#
# Default rules...
#
.c.o:
	$(CC) $(CFLAGS) $*.c

.asm.o:
	$(AS) $*.asm $(AFLAGS) $*.o

#
# Specific rules
#
setalert.o:	setalert.c	$(MODNAME)_rev.h	setalert_strings.h

setalert_strings.h:	setalert.cd
	CatComp setalert.cd CFILE setalert_strings.h OBJFILE setalert_strings.o CTFILE setalert.ct

setalert_strings.o:	setalert.cd
	CatComp setalert.cd CFILE setalert_strings.h OBJFILE setalert_strings.o CTFILE setalert.ct

#
# Bump the version...
#
version:
	Bumprev $(VERSION) $(MODNAME)_rev

$(MODNAME)_rev.h:
	Bumprev $(VERSION) $(MODNAME)_rev

#
# Blow away all files that would need to be rebuilt
#
clean:
	Delete \#?.(o|ld|doc|map|ct) \#?_strings.(o|h|i)

#
# Make the AutoDocs
#
doc:	$(MODNAME).doc

$(MODNAME).doc:	$(MY_C) $(MY_A)
	Echo "Making $(MODNAME).doc"
	Join $(MY_C) $(MY_A) as RAM:$(MODNAME).tmp.doc
	AutoDoc >$(MODNAME).doc RAM:$(MODNAME).tmp.doc
	Delete RAM:$(MODNAME).tmp.doc QUIET
	Protect $(MODNAME).doc rwd

#
# To build a release we do the following...
#
release:
	smake -s clean
	smake -s version
	smake -s doc
	smake -s
