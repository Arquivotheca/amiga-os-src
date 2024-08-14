
INCLUDE	= -i//amiganet
CFLAGS  = -rr -v $(INCLUDE) -O
BINARIES= datatest debug badargs send recv prime nbtest

all:	$(BINARIES)
	
badargs: badargs.c
	lc $(INCLUDE) -rr -v -d5 -L -O badargs
	-delete badargs.info

debug: debug.c
	lc $(INCLUDE) -rr -v -d5 -L -O debug
	-delete debug.info

datatest: datatest.c
	lc $(INCLUDE) -rr -v -d5 -L -O datatest
	-delete datatest.info


send: send.c
	lc $(INCLUDE) -rr -v -d5 -L -O send
	-delete send.info


recv: recv.c
	lc $(INCLUDE) -v -d5 -L recv
	-delete recv.info


nbtest: nbtest.c
	lc $(INCLUDE) -v -d5 -L nbtest
	-delete nbtest.info


prime: prime.c
	lc $(INCLUDE) -v -d5 -L prime
	-delete prime.info


# clean directory for backup
clean:
	-delete \#?.o
	-delete \#?.info
	-delete \#?.lnk

spotless: clean
	-delete $(BINARIES)
