# =========================================================================
# Makefile for VU using SAS C 6.1
# =========================================================================

.c.o:
	SC $*.c nolink opt optsize nostdio nodebug

MAINOBJS	= vu.o images.o

OBJS		= $(MAINOBJS)

fta: $(OBJS)
	SC $(OBJS) ProgramName=vu stripdebug
