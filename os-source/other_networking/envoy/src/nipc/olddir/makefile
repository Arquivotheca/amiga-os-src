# $Id: makefile,v 1.17 92/06/09 15:10:08 gregm Exp $
#
# nipc.library's makefile
# I've switched to Manx make because lmk sucks, and my copy of dmake is buggy.
# I also tried the bmake that Martin gave me, but tossed it because it had a weird
# requirement that command lines have to begin with a TAB character.  I [gregm]
# use DME, which expands tabs, so f*ck it.
#

COBJS = center_custom.o memory.o s2io.o arp.o ip_out.o route.o ip_in.o icmp.o timer.o rdp.o amp.o statistics.o udp.o resolver.o
AOBJS = nipcface.o callback.o checksum.o nipcproc.o iff.o math.o

all : nipc.library

nipcend.o : nipcend.asm
            casm -a $*.asm -cqy -iinc: -o$*.o

nipcface.o : nipc.library_rev.i nipcbase.i
center_custom.o : center_custom.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
memory.o : memory.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
s2io.o : s2io.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
arp.o : arp.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
ip_out.o : ip_out.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
route.o : route.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
ip_in.o : ip_in.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
icmp.o : icmp.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
timer.o : timer.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
rdp.o : rdp.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
amp.o : amp.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
statistics.o : statistics.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
monitor.o : monitor.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
udp.o : udp.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
resolver.o : resolver.c nipcinternal.h nipc.h nipcinternal_protos.h externs.h nipcbase.h
iff.o : iff.asm nipcbase.i
math.o : math.asm

fd/nipc_lib.fd: nipc_lib.sfd
        sfd nipc_lib.sfd
        copy FD/nipc_lib.fd nipc_lib.fd
        copy PROTOS/nipc_protos.h INCLUDE:clib
        copy PRAGMAS/nipc_pragmas.h INCLUDE:pragmas


$(COBJS)       :
                     lc -. -b0 -d1 -rr -mes -cfist -v -O $*.c


$(AOBJS)       :
                     casm -a $*.asm -cqy -iinc: -o$*.o

nipc.library   : $(COBJS) $(AOBJS) nipcend.o fd/nipc_lib.fd
                 blink $(AOBJS) $(COBJS) nipcend.o to nipc.library NODEBUG LIB lib:lcr.lib lib:small.lib lib:debug.lib
                 copy nipc.library libs:

#Note that because we don't have MakeMeta native, we need to hand generate our
#own stubs manually when something changes.

release: nipc.library fd/nipc_lib.fd
        copy FD/nipc_lib.fd envoy:fd NOPRO
        copy PROTOS/nipc_protos.h envoy:include/clib NOPRO
        copy PROTOS/nipc_protos.h ""
        copy PRAGMAS/nipc_pragmas.h envoy:include/pragmas NOPRO
        copy PRAGMAS/nipc_pragmas.h ""
        copy nipc.h envoy:include/envoy NOPRO
        copy nipc.h include:envoy NOPRO
        copy RAM envoy:src/stubs/ NOPRO all
        copy nipc.library envoy:disk/envoy/libs/ NOPRO
        copy #?.c #?.asm #?.h #?.rev nipc.library makefile envoy:src/nipc NOPRO

clean:
        touch tmp
        delete $(COBJS) $(AOBJS) tmp libent.o libinit.o nipc.library
        touch tmp
        delete tmp fd lvo hdr protos pragmas ram rom all

bump:
         co -l nipc.library_rev.h
         co -l nipc.library_rev.i
         co -l nipc.library_rev.rev
         bumprev 37 nipc.library_rev
         ci -u -m"" nipc.library_rev.h
         ci -u -m"" nipc.library_rev.i
         ci -u -m"" nipc.library_rev.rev
         touch nipcface.asm
