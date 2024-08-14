
CFLAGS=+l -s -n
JANUS=sys1:lib/janus.lib
DETACH=sys2:lib/detach.o32

Progs:	AMouse Dump
	echo >Progs Done

AMouse: AMouse.o JMouse.o TellUser.o
	ln -t -g AMouse.o JMouse TellUser $(DETACH) +l $(JANUS) +l -lc32

JMouse.o: JMouse.asm myjanus.i
	as -EMANX JMouse

AMouse.o: sys1:include/janus/setupsig.h

Dump: Dump.o
	ln -t -g Dump -lc32

