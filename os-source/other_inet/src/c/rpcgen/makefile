# -----------------------------------------------------------------------
# MAKEFILE for RPCGEN (Manx 36)
#
# $Locker:  $
#
# $Id: makefile,v 1.3 90/11/12 19:32:53 martin Exp $
#
# $Revision: 1.3 $
#
# $Header: inet2:src/c/rpcgen/RCS/makefile,v 1.3 90/11/12 19:32:53 martin Exp $
#
#------------------------------------------------------------------------
 

#
# @(#)Makefile	1.2 87/11/09 3.9 RPCSRC
#
# Makefile for rpc protocol compiler
# Copyright (C) 1987, Sun Microsystems, Inc.
#

CLIB	= c32.lib
NETLIB	= NETLIB:socket32.lib
NETBIN  = NETBIN:c
NETINC  = NETINC:

SRCS	= rpc_main.c rpc_hout.c rpc_cout.c rpc_parse.c rpc_scan.c rpc_util.c \
	  rpc_svcout.c rpc_clntout.c
HDRS	= rpc_util.h rpc_parse.h rpc_scan.h
OBJS	= rpc_main.m rpc_hout.m rpc_cout.m rpc_parse.m rpc_scan.m rpc_util.m \
	  rpc_svcout.m rpc_clntout.m cpp.m 
BIN	= inet:c
CFLAGS 	=  +l -I$(NETINC) -DAMIGA

.c.m:
	cc +l -DAMIGA -I$(NETINC) -o $*.m $*.c
	
.s.m:	
    as -o $*.m $*.s
    
all:	rpcgen gnucpp

rpcgen: rpcgenversion.m 
	ln  +ss -m +q -o $@ rpcgenversion.m $(OBJS) $(NETLIB) $(CLIB) $(CLIB)

rpcgenversion.m: $(OBJS)
	vers >rpcgenversion.c -v rpcgen
	cc -o rpcgenversion.m rpcgenversion.c

GNUOBJ	= cccp.m cexp.m cpp.m alloca.m
gnucpp:	cppvers.m
	ln +ss -m +q -w -o $@ cppvers.m $(GNUOBJ) $(CLIB) $(CLIB)

cppvers.m: $(GNUOBJ)
	vers >cppvers.c -v gnucpp
	cc -o cppvers.m cppvers.c

alloca.m: alloca.s
	as -o alloca.m alloca.s

clean:
	delete #?.m

install: rpcgen gnucpp
	copy rpcgen $(NETBIN) clone
	copy gnucpp $(NETBIN) clone

bump:
	bumprev 36 rpcgen_vers
	bumprev 36 gnucpp_vers

