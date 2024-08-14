SUBDIRS=	bcpl calink cassem sysl
EXECUTABLES=	absload.out atom.out genlib.out makeinclude.out \
		symba.out symbm.out

.c.out:
	rcp $? ghostwheel:qwe.$?
	rsh ghostwheel cc -O qwe.$? -o qwe.$@
	rcp ghostwheel:qwe.$@ $@
	rsh ghostwheel rm qwe.$? qwe.$@

all:		${SUBDIRS} ${EXECUTABLES}

install:
	for i in ${SUBDIRS}; do cp $$i/$$i ../bin; done;
	for i in ${EXECUTABLES}; do cp $$i ../bin/`basename $$i .out`; done;

${SUBDIRS}: FRC
	cd $@; make ${MFLAGS} ${MARGS}

FRC:
