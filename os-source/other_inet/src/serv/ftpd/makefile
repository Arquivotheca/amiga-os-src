#
# Makefile for ftpd
#

CFLAGS	= +l -Inetinc: -DAMIGA
CLIB	= c32.lib
NETLIB	= NETBIN:lib/socket32.lib
SRCS	= ftpd.c ftpcmd.c glob.c logwtmp.c popen.c vers.c
OBJS	= ftpd.o glob.o vers.o strpbrk.o getusershell.o ftpcmd.o compat.o \
	  logwtmp.o
BIN	= NETBIN:serv

all: ftpd

ftpcmd.c:	ftpcmd.y
	delete ftpcmd.c
	yacc ftpcmd.y
	rename y.tab.c ftpcmd.c

ftpd: $(OBJS) 
	ln +q -m -o $@ $(OBJS) $(NETLIB) $(CLIB)

clean:
	delete #?.o

install:	ftpd
	copy ftpd $(BIN) clone
