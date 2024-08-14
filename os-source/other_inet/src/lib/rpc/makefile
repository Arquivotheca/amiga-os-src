#
# Makefile for RPC library.
#

MRPC32 =svc_run.m svc_simple.m svc_tcp.m svc_udp.m xdr_stdio.m clnt_generic.m \
	clnt_raw.m clnt_simple.m getrpcent.m getrpcport.m pmap_getmaps.m \
	clnt_tcp.m xdr_rec.m pmap_prot2.m pmap_rmt.m xdr_reference.m \
	auth_unix.m rpc_commondata.m svc.m pmap_clnt.m clnt_perror.m \
	get_myaddress.m svc_auth.m svc_auth_unix.m authunix_prot.m \
	xdr_array.m svc_raw.m rpc_callmsg.m pmap_getport.m clnt_udp.m \
	pmap_getport.m clnt_udp.m auth_none.m xdr_mem.m pmap_prot.m \
	rpc_dtablesize.m rpc_prot.m xdr.m

LRPC32 =svc_run.l svc_simple.l svc_tcp.l svc_udp.l xdr_stdio.l clnt_generic.l \
	clnt_raw.l clnt_simple.l getrpcent.l getrpcport.l pmap_getmaps.l \
	clnt_tcp.l xdr_rec.l pmap_prot2.l pmap_rmt.l xdr_reference.l \
	auth_unix.l rpc_commondata.l svc.l pmap_clnt.l clnt_perror.l \
	get_myaddress.l svc_auth.l svc_auth_unix.l authunix_prot.l \
	xdr_array.l svc_raw.l rpc_callmsg.l pmap_getport.l clnt_udp.l \
	pmap_getport.l clnt_udp.l auth_none.l xdr_mem.l pmap_prot.l \
	rpc_prot.l xdr.l gettimeofday.l bindresvport.l requester.l

NOGO=	xdr_float.o

all:	rpcS.lib

.c.m:
	cc +l +B -DAMIGA -I//include $*.c -o $@

rpc32.lib:	$(MRPC32)
	lb $@  $(MRPC32)

.c.l:
	lc -b1 -v -cw -co -DAMIGA -inetinc: -O -o$*.l $*.c

rpcS.lib:	$(LRPC32)
	list >oml.list quick nohead #?.l
	oml -s rpcS.lib @oml.list
	-delete oml.list

clean:
	-delete #?.(l|m)

install: rpcS.lib
