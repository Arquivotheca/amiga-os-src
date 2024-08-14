# The source code to sfd is a lex file (sfd.l), which exists only
# on the Sun, and is converted using lex running on the Sun.  The
# resulting sfd.c file may be compiled on the Sun or on the Amiga.
#
# The Amiga version of sfd is found in V:aug/src/sfd, along with a
# makefile which grabs sfd.c from here and compiles it.

# This rule builds the sfd.c file using lex, then builds the Sun-based
# executable, calling it sfd.out.
sun2:
	@echo "You better be building this on a Sun 2,"
	@echo "or else it won't execute on a Sun 2!"
	lex -t sfd.l >sfd.c
	cc -O -o sfd.out sfd.c

# This rule installs the executable and doc into V:tools/bin and
# V:tools/doc.

install:
	rm -f sfd.test

	rm -f ../../bin/sfd
	cp sfd.out ../../bin/sfd

	rm -f ../../doc/sfd.doc
	cp sfd.doc ../../doc

test:
	lex -t sfd.l >sfd.c
	cc -O -DDEBUG sfd.c -o sfd.test

