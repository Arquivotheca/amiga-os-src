#
# $Id: $
#
# Utility.library NMakeMeta makefile for amake
# Copyright 1990, Commodore Amiga, Inc.
#

# defines the compiler stuff, must come FIRST
.INCLUDE=NMM:NCOMPILERS

# If you want your objects in a subdirectory, use compiler options, and
# specify ${OBJ} for each file in OFILES.
# (DFLAGS is for the autodoc program)

CFLAGS		=	-. -b0 -v -d -cwus -ml
AFLAGS		=	-cvrf -iinclude:
LFLAGS		=	BATCH BUFSIZE 1024 QUIET SC SD
DFLAGS		=
ASSEMBLER	=	${CAPE}
COMPILER	=	${LATTICE}
CLEANUP		=
OBJ		=

# who are we?

MODULE		=	utility

# this must be a COMPLETE path!!!!!!!

MODULEDIR	=	V:src/kickstart/${MODULE}

# all our source files

AFILES		=	ulib.asm ufunctions.asm callhook.asm math.asm \
			datetools.asm stricmp.asm
CFILES		=	tagitem.c hardware.c

# don't forget to update this when you change CFILES or AFILES!!!
# see above about subdirectories

OFILES		=	ulib.o ufunctions.o callhook.o math.o \
			datetools.o stricmp.o tagitem.o hardware.o

# External include files - by default they go in ${MODULE}/*.
# If the specification has a / in it, it's relative to include.nostrip:,
# else it's relative to include.nostrip:${module} (or include.strip: etc).
# NOTE: this means to do this, they MUST be in a subdirectory!

EXTINCL		=	tagitem.h hooks.h date.h utility.i tagitem.i \
			hooks.i date.i

# all .h and .i files we care about

HFILES		=	tagitem.h hooks.h date.h ubase.h
IFILES		=	utility.i tagitem.i hooks.i date.i ubase.i asmsupp.i

# internal-only includes (they go in include.{nostrip,strip}/internal/xxxx)

INTINCL		=

# special libraries we need to link with (other than amiga.lib)

LIBS		=	LIB:lc.lib

# additional files (other than AFILES, CFILES, IFILES, and HFILES) to be kept
# under RCS.

RFILES		= makefile



# This must come just before the all: line!
.INCLUDE=NMM:NLINKERS

LINKER	=	${LLATTICE}

# the key line...

all : ${MODULE}.ld



# other dependancies for files

ulib.o : ulib.asm ${MODULE}_rev.i

#	special handling for lattice assembled modules
#	(ALATTICE includes -iINCLUDE:)
math.o : math.asm
	'${ALATTICE} ${AFLAGS} -m2 math.asm'

datetools.o : datetools.asm
	'${ALATTICE} ${AFLAGS} -m2 datetools.asm'



####
# backup source on my amiga to cbmvax for safety - REJ

backup :
	'copy "" vax:${MODULE} all nopro'

# This should be the last thing in the file
.INCLUDE=NMM:NMakeMeta
