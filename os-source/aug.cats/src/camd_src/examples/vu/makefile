# =========================================================================
# Makefile for VU using SAS C 6.1
# =========================================================================

.c.o:
	SC $*.c opt optsize nostdio nodebug

MAINOBJS	= vu.o images.o

OBJS		= $(MAINOBJS)

vu: $(OBJS)
        SLINK lib:c.o $(OBJS) noicons library lib:sc.lib lib:amiga.lib to vu
	
