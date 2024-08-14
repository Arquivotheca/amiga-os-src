#---------------------------------------------------------------------------#
# Makefile for: telnet.c   Manx in this case                                #
#                                                                           #
# $Locker$                                                                  #
#                                                                           #
# $Id$                                                                      #
#                                                                           #
# $Revision$                                                                #
#                                                                           #
# $Header$                                                                  #
#                                                                           #
#---------------------------------------------------------------------------#
# Makefile for telnet                                                       #
#---------------------------------------------------------------------------#
                                                                             
NETBIN = NETBIN:c
NETINC = NETINC:
NETLIB = NETLIB:socketm.lib
CLIB   = c.lib
BIN    = inet:c
OBJ    = telnet.m commands.m tmain.m network.m ring.m terminal.m utilities.m
TLIB   = telnet.lib
TELOBJ = sys_amiga.m

.c.m:
	cc  -i $(NETINC) -o$*.m $*.c

all:	telnet

telnet:	version.m $(OBJ)
	ln -m +q -o $@ version.m $(TELOBJ) $(TLIB) $(NETLIB) $(CLIB)

telnet.lib: $(OBJ)
	lb telnet.lib $(OBJ)

version.m: $(TLIB) $(TELOBJ)
	vers >version.c -v telnet
	cc -oversion.m version.c
	

clean:
	delete #?.m
	delete #?.o

install:	telnet
	copy telnet $(BIN)
	copy telnet $(NETBIN)

bump:
	bumprev 37 telnet_rev
