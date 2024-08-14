#
#
# What is this? A test makefile?
#
#

RFILES		=	NCOMPILERS NMakeMeta
MODULE		=	NativeMakeMeta
MODULEDIR	=	V36:NativeMakeMeta
CFILES		=	
OFILES		=	
LIBS		=	
CLEANUP		=	temp|t1

.INCLUDE=NCOMPILERS

COMPILER=${LATTICE}
ASSEMBLER=${ALATTICE}
LINKER=${LLATTICE}

all : ${MODULE}.ld

NMakeMeta.dup : NMakeMeta
	'copy NMakeMeta NMakeMeta.dup'

.INCLUDE=NMakeMeta.dup

