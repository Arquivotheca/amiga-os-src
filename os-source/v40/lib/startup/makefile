######################################################################
MAKEMETA=	../../tools/makemeta

IFILES=	Astartup.i AWstartup.i Rstartup.i RWstartup.i RXstartup.i \
	Qstartup.i

all quick:	FRC bcpl-startup.obj
	for i in ${IFILES} ; do \
	    ( ${ASM} startup.asm -h $$i -o startup.obj -c q,a${SYMBOLOPT} \
		    ${AFLAGS} -i \"${MYINCL} ${SYSINCLNS}\"; \
	    echo $$i: | sed -e "s/i/obj/"; \
	    mv startup.obj ../`echo $$i | sed -e "s/i/obj/"` ); done;
	mv bcpl-startup.obj ..

bcpl-startup.obj:	bcpl-startup.asm

.INCLUDE=${MAKEMETA}
