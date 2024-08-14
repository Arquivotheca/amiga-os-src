#$Header$
#
#
#
#
#
.SILENT:

CC1=lc1
CC2=go
CC3=lc2
C1FLAGS	= -d3
C2FLAGS	=
C3FLAGS	=

#.c.o:
#	$(CC1) -. $(C1FLAGS) $(CFLAGS) -oQUAD: $*
#	$(CC2) -. $(C2FLAGS) QUAD:$*.q
#	$(CC3) -. $(C3FLAGS) -o$*.o QUAD:$*.q
#
.c.o:
	$(CC1) -. $(C1FLAGS) $(CFLAGS) -oQUAD: $*
	$(CC3) -. $(C3FLAGS) -o$*.o QUAD:$*.q
#
#

LDFLAGS	= quiet batch
LDLIBS	= LIB:rsbx.lib LIB:lc.lib LIB:amiga.lib

CDEFINES =
CFLAGS   = $(CDEFINES)

TARGETS = cmp


all:	$(TARGETS)


install:
	copy cmp to sys:utilities/unix


clean:
	-delete \#?.o
	-delete $(TARGETS)
	-delete \#?.tmp


CMPOBJ = cmp.o
cmp: $(CMPOBJ) $(LDLIBS)
	-delete $@
	${LD} $(LDFLAGS) TO $@.tmp FROM LIB:xc.o $(CMPOBJ) LIB $(LDLIBS)
	rename $@.tmp $@


SOURCE= cmp.c

depend: ${SOURCE}
	/mdepend/mdepend Makefile ${SOURCE}
	-protect Makefile.bak +d
	copy Makefile Makefile.bak
	-protect Makefile +d
	copy Makefile.new Makefile
	delete Makefile.new


# DO NOT DELETE THIS LINE, REMAINDER GENERATED AUTOMATICALLY
cmp.o: cmp.c
cmp.o: INCLUDE:stdio.h
