#
# makefile for NFS server
#
CC	= lc
CFLAGS	= -v -cfist -O -inetinc:
RPCLIB	= as225:bin/lib/rpcS.lib
CLIB	= lib:amiga.lib lib:lc.lib 
RPCSVC	= as225:bin/lib/rpcsvcS.lib
BIN	= inet:serv

all: 	NFSd

clean:
	delete #?.o nfsd.dbg

.c.o:
	$(CC) $(CFLAGS) $*.c

.a.o:
	asm -u -iINCLUDE: $*.a

.asm.o:
#	casm -a $*.asm -cqvy -o $*.o -iwork:cape/includes
	casm -a $*.asm -cqvy -o $*.o -iinclude:


NFSOBJ= \
	mount_svc.o mntd.o xdr.o fh.o main.o \
	read.o write.o remove.o  \
	setattr.o statfs.o link.o mount.o \
	nfserr.o cache.o portmap.o svc_run.o bitmap.o \
	nfs_io.o map.o build_vol.o rexxglue.o rexxvars.o

obj.lib:	$(NFSOBJ)
	oml obj.lib $(NFSOBJ)

NFSd: obj.lib c.o nfs_svc.o create.o lookup.o  readdir.o getattr.o rename.o nfstime.o 
	blink from c.o nfs_svc.o create.o lookup.o readdir.o getattr.o lib obj.lib \
		rename.o nfstime.o $(RPCSVC) $(RPCLIB) lib:debug.lib $(CLIB) \
		to NFSd SC SD ND

rexxvars.o: rexxvars.lib
	copy rexxvars.lib $@

install: NFSd
	copy nfsd $(BIN) clone

bump:
	bumprev 3 nfsd_rev
