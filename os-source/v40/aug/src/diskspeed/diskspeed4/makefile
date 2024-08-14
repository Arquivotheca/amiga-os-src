#
#                          DiskSpeed v4.2
#                          ScsiSpeed v4.2
#                                by
#                           Michael Sinz
#
#             Copyright (c) 1989-1992 by MKSoft Development
#
#			MKSoft Development
#			163 Appledore Drive
#			Downingtown, PA 19335
#
# Yes, this is yet another disk speed testing program, but with a few
# differences.  It was designed to give the most accurate results of the
# true disk performance in the system.  For this reason many of
# DiskSpeed's results may look either lower or higher than current disk
# performance tests.
#
##############################################################################
#									     #
#	Reading legal mush can turn your brain into guacamole!		     #
#									     #
#		So here is some of that legal mush:			     #
#									     #
# Permission is hereby granted to distribute this program's source	     #
# executable, and documentation for non-commercial purposes, so long as the  #
# copyright notices are not removed from the sources, executable or	     #
# documentation.  This program may not be distributed for a profit without   #
# the express written consent of the author Michael Sinz.		     #
#									     #
# This program is not in the public domain.				     #
#									     #
# Fred Fish is expressly granted permission to distribute this program's     #
# source and executable as part of the "Fred Fish freely redistributable     #
# Amiga software library."						     #
#									     #
# Permission is expressly granted for this program and it's source to be     #
# distributed as part of the Amicus Amiga software disks, and the	     #
# First Amiga User Group's Hot Mix disks.				     #
#									     #
##############################################################################
#
# MakeFile for DiskSpeed and ScsiSpeed
#

#
# ASM from Lattice
.asm.o:
	asm -iInclude: $*.asm

CFLAGS= -b1 -cfistq -d2 -ms0 -v -rr1 -O

.c.o:
	@LC $(CFLAGS) $*

OBJS=	DiskSpeedCPU.o RenderInfo.o MakeBoxes.o MKS_list.o

LIBS=	LIB:lcr.lib LIB:small.lib

ALL:	DiskSpeed ScsiSpeed

DiskSpeed: DiskSpeed.ld
	BLink FROM DiskSpeed.ld to DiskSpeed ND

DiskSpeed.ld: DiskSpeed.o $(OBJS) $(LIBS)
	BLink FROM LIB:c.o DiskSpeed.o $(OBJS) TO DiskSpeed.ld LIB $(LIBS) DEFINE @_main=@_tinymain SD SC

ScsiSpeed: ScsiSpeed.ld
	BLink FROM ScsiSpeed.ld to ScsiSpeed ND

ScsiSpeed.ld: ScsiSpeed.o $(OBJS) $(LIBS)
	BLink FROM LIB:c.o ScsiSpeed.o $(OBJS) TO ScsiSpeed.ld LIB $(LIBS) DEFINE @_main=@_tinymain SD SC

DiskSpeed.o:	DiskSpeed.c DiskSpeed_rev.h
ScsiSpeed.o:	ScsiSpeed.c DiskSpeed.c ScsiSpeed_rev.h

version:
	BumpRev 4 DiskSpeed_rev
	BumpRev 4 ScsiSpeed_rev
