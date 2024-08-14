#
# RPC services makefile.
#

RPCGEN	= rpcgen

.x.h:
	$(RPCGEN) -h $*.x -o $@
.x.c:
	$(RPCGEN) -c $*.x -o $@

.c.l:
	lc -b1 -v -cw -co $(CFLAGS) -inetinc: -O -o$*.l $*.c

.c.m:
	cc +l +B -I//include $*.c -o $@

all:	rpcsvc32.lib

nfs.c:		nfs.x nfs.h
mount.c:	mount.x mount.h
yp.c:		yp.x yp.h
bootparam_prot.c:	bootparam_prot.x bootparam_prot.h

nfs.h:		nfs.x
mount.h:	mount.x
yp.h:		yp.x
bootparam_prot.h:	bootparam_prot.x

OBJ32	= nfs.m mount.m yp.m bootparam_prot.m
LOBJ32	= nfs.l mount.l yp.l bootparam_prot.l

rpcsvcS.lib:	$(LOBJ32)
	oml rpcsvcS.lib r $(LOBJ32)

rpcsvc32.lib:	$(OBJ32)
	lb $@ $(OBJ32)

install:	mount.h yp.h nfs.h rpcsvcS.lib
	copy mount.h 	netinc:rpcsvc
	copy yp.h 	netinc:rpcsvc
	copy nfs.h 	netinc:nfs
	copy bootparam_prot.h netinc:rpcsvc

clean:
	-delete #?.l #?.m
	-delete nfs.c mount.c yp.c
