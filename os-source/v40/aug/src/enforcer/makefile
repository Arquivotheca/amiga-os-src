#
# Makefile for Enforcer
#

#
# Server directory for the data...
#
REMOTE = V:aug/src/enforcer
REMOTEBIN = V:aug/bin
REMOTEDOC = V:aug/doc
VERSION = 37

#
# Name of the module
#
MODNAME	= Enforcer
MODNAME1= LawBreaker
MODNAME2= Move4K
MODNAME3= SegTracker
MODNAME4= FindHit
MODNAME5= RebootOff

#
# The ".LD" file we want to produce.
#
LD_FILE		= $(MODNAME).ld
LD_STRIP	= $(MODNAME)

LD1_FILE	= $(MODNAME1).ld
LD1_STRIP	= $(MODNAME1)

LD2_FILE	= $(MODNAME2).ld
LD2_STRIP	= $(MODNAME2)

LD3_FILE	= $(MODNAME3).ld
LD3_STRIP	= $(MODNAME3)

LD4_FILE	= $(MODNAME4).ld
LD4_STRIP	= $(MODNAME4)

LD5_FILE	= $(MODNAME5).ld
LD5_STRIP	= $(MODNAME5)

ALL:	$(LD_STRIP)	$(LD1_STRIP)	$(LD2_STRIP)	$(LD3_STRIP)	$(LD4_STRIP)	$(LD5_STRIP)

#
# Compiler, assembler, and linker
#
CC = sc
AS = hx68 -a
LN = slink
LN1= blink

#
# Compiler flags...
#
CFLAGS = IGNORE=306 MEMORYSIZE=HUGE DATA=FARONLY PARAMETERS=REGISTER NOSTACKCHECK NOMULTIPLEINCLUDES STRINGMERGE STRUCTUREEQUIVALENCE MULTIPLECHARACTERCONSTANTS DEBUG=LINE NOVERSION OPTIMIZE OPTIMIZERINLINE OPTIMIZERINLOCAL NOICONS

#
# Assembler flags...  NOTE how they are used below...
#
AFLAGS	= -csq -i include: -o

#
# Linker flags
#
LFLAGS		= SMALLCODE SMALLDATA MAP $(MODNAME).map HLSXF PLAIN SWIDTH 16 NOICONS QUIET
LFLAGS1		= SMALLCODE SMALLDATA MAP $(MODNAME1).map HLSXF PLAIN SWIDTH 16 NOICONS QUIET
LFLAGS2		= SMALLCODE SMALLDATA MAP $(MODNAME2).map HLSXF PLAIN SWIDTH 16 NOICONS QUIET
LFLAGS3		= SMALLCODE SMALLDATA MAP $(MODNAME3).map HLSXF PLAIN SWIDTH 16 NOICONS QUIET
LFLAGS4		= SMALLCODE SMALLDATA MAP $(MODNAME4).map HLSXF PLAIN SWIDTH 16 NOICONS QUIET
LFLAGS5		= SMALLCODE SMALLDATA MAP $(MODNAME5).map HLSXF PLAIN SWIDTH 16 NOICONS QUIET
LFLAGS_STRIP	= ND NOICONS

#
# System libraries
#
LIBS= LIB:amiga.lib

#
# My headers...
#
MY_HDR	=

#
# My C source...
#
MY_C	=	enforcer.c	segtracker.c	findhit.c

#
# My Assebly source...
#
MY_A	=	handler.asm lawbreaker.asm move4k.asm segtrackerpatch.asm rebootoff.asm

#
# My object files...
#
MY_OBJS	=	enforcer.o handler.o
MY_OBJS1=	lawbreaker.o
MY_OBJS2=	move4k.o
MY_OBJS3=	segtracker.o segtrackerpatch.o
MY_OBJS4=	findhit.o
MY_OBJS5=	rebootoff.o

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
	$(LN1) FROM $(LD_FILE) TO $(LD_STRIP) $(LFLAGS_STRIP)

# LawBreaker has debugging info in it for example use...
$(LD1_STRIP):	$(LD1_FILE)
	$(LN1) FROM $(LD1_FILE) TO $(LD1_STRIP)

$(LD2_STRIP):	$(LD2_FILE)
	$(LN1) FROM $(LD2_FILE) TO $(LD2_STRIP) $(LFLAGS_STRIP)

$(LD3_STRIP):	$(LD3_FILE)
	$(LN1) FROM $(LD3_FILE) TO $(LD3_STRIP) $(LFLAGS_STRIP)

$(LD4_STRIP):	$(LD4_FILE)
	$(LN1) FROM $(LD4_FILE) TO $(LD4_STRIP) $(LFLAGS_STRIP)
	@Protect $(LD4_STRIP) +p

$(LD5_STRIP):	$(LD5_FILE)
	$(LN1) FROM $(LD5_FILE) TO $(LD5_STRIP) $(LFLAGS_STRIP)

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

$(LD1_FILE):	$(MY_OBJS1) $(MY_LIBS)
	$(LN) <with <
FROM $(MY_OBJS1)
TO $(LD1_FILE)
LIB $(MY_LIBS) $(LIBS)
$(LFLAGS1)
<

$(LD2_FILE):	$(MY_OBJS2) $(MY_LIBS)
	$(LN) <with <
FROM $(MY_OBJS2)
TO $(LD2_FILE)
LIB $(MY_LIBS) $(LIBS)
$(LFLAGS2)
<

$(LD3_FILE):	$(MY_OBJS3) $(MY_LIBS)
	$(LN) <with <
FROM $(MY_OBJS3)
TO $(LD3_FILE)
LIB $(MY_LIBS) $(LIBS)
$(LFLAGS)
<

$(LD4_FILE):	$(MY_OBJS4) $(MY_LIBS)
	$(LN) <with <
FROM $(MY_OBJS4)
TO $(LD4_FILE)
LIB $(MY_LIBS) $(LIBS)
$(LFLAGS)
<
	@Protect $(LD4_FILE) +p

$(LD5_FILE):	$(MY_OBJS5) $(MY_LIBS)
	$(LN) <with <
FROM $(MY_OBJS5)
TO $(LD5_FILE)
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
handler.o:	handler.asm	$(MODNAME)_rev.i

lawbreaker.o:	lawbreaker.asm	$(MODNAME1)_rev.i

move4k.o:	move4k.asm	$(MODNAME2)_rev.i

segtracker.o:	segtracker.c	$(MODNAME3)_rev.h

findhit.o:	findhit.c	$(MODNAME4)_rev.h

rebootoff.o:	rebootoff.asm	$(MODNAME5)_rev.i

#
# Bump the version...
#
version:
	@bumprev $(VERSION) $(MODNAME)_rev
	@bumprev $(VERSION) $(MODNAME1)_rev
	@bumprev $(VERSION) $(MODNAME2)_rev
	@bumprev $(VERSION) $(MODNAME3)_rev
	@bumprev $(VERSION) $(MODNAME4)_rev
	@bumprev $(VERSION) $(MODNAME5)_rev

$(MODNAME)_rev.i:
	@bumprev $(VERSION) $(MODNAME)_rev

$(MODNAME1)_rev.i:
	@bumprev $(VERSION) $(MODNAME1)_rev

$(MODNAME2)_rev.i:
	@bumprev $(VERSION) $(MODNAME2)_rev

$(MODNAME3)_rev.h:
	@bumprev $(VERSION) $(MODNAME3)_rev

$(MODNAME4)_rev.h:
	@bumprev $(VERSION) $(MODNAME4)_rev

$(MODNAME5)_rev.i:
	@bumprev $(VERSION) $(MODNAME5)_rev

#
# Blow away all files that would need to be rebuilt
#
clean:
	@delete \#?.(o|ld|doc|ps|map|map.info|ld.info|o.info)

#
# Make the AutoDocs
#
doc:	$(MODNAME).doc

$(MODNAME).doc:	$(MY_C) $(MY_A)
	@Echo "Making $(MODNAME).doc"
	@Join $(MY_C) $(MY_A) as RAM:$(MODNAME).tmp.doc
	@AutoDoc >$(MODNAME).doc RAM:$(MODNAME).tmp.doc
	@Delete RAM:$(MODNAME).tmp.doc QUIET
	@Protect $(MODNAME).doc rwd
