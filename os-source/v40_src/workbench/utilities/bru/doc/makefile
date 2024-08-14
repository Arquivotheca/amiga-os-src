#
#  FILE
#
#	Makefile    makefile for bru documentation directory
#
#  SCCS
#
#	@(#)Makefile	5.6	3/20/89
#
#  SYNOPSIS
#
#	cd bru/doc; make [targets]
#
#  DESCRIPTION
#
#	This is the makefile which controls building and installation
#	of the bru documentation.  It is normally invoked by a recursive
#	make in the bru root directory via the bru.mk makefile.
#

.SUFFIXES:	.t .t~ .n .n~ .1 .1~ .man

.n~.n:
		$(GET) $(GFLAGS) -p $< >$*.n

.t~.t:
		$(GET) $(GFLAGS) -p $< >$*.t

.1.man:
		tbl $< | nroff -man >$*.man

.1~.man:
		$(GET) $(GFLAGS) -p $< >$*.n
		tbl $*.1 | nroff -man >$*.man
		-rm -f $*.1

.n.t:
		tbl $< | nroff -mm >$*.t

.n~.t:
		$(GET) $(GFLAGS) -p $< >$*.n
		tbl $*.n | nroff -mm >$*.t
		-rm -f $*.n

#
#	Make the documentation
#

default :	bru.man

#
#	Clean up by removing various things.
#

clean :
		rm -f nohup.out *.BAK *.old *.bak
