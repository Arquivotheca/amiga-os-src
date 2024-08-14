#
# Makefile for network support library
#

MOBJ =stat.m getdomain.m inet_makeaddr.m getnetbyaddr.m 

OBJL =	socket.l getwd.l stat.l popen.l dir.l syslog.l sleep.l isatty.l access.l


PROBS	= fstat.l

#
# make the autodocs
#

docs:
	join #?.(c|asm) as ram:temp_docs
	autodoc >docs -t4 ram:temp_docs
	delete ram:temp_docs

.asm.m:
	as $*.asm -o $*.m

.c.m:
	cc -so -wo -inetinc: -DAMIGA $*.c -o $*.m

all:	unixM.lib unixS.lib

unixM.lib: $(MOBJ)
	lb $@ $(MOBJ)

clean:
	delete #?.l #?.m

install: unixS.lib 
	copy #?.lib inet:lib clone

#
# SAS format unixS.lib
#

.asm.l:
	casm -A $*.asm -o $*.l

.c.l:
	lc -O -cfist -v -DAMIGA -inetinc: -o$*.l $*.c

unixS.lib:	$(OBJL)
	list >oml.list nohead quick #?.l
	oml unixS.lib @oml.list
	delete oml.list
