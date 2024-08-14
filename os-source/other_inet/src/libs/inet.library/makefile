#
# makefile for inet.library.  In Manx makefile syntax.
#

#CFLAGS = -DAMIGA -DBSD=43 -DKERNEL -DINET -DGATEWAY

CFLAGS = -DAMIGA -DBSD=43 -DKERNEL -DINET

.c.l:
        lc -ml -d1 -b1 -v -cfist -O $(CFLAGS) -iNETINC: -o$*.l $*.c

.c.o:
        lc -ml -d1 -b1 -v -cfist -O $(CFLAGS) -iNETINC: -o$*.o $*.c

.c.m:
        cc -so -wo $(CFLAGS) -iNETINC: $*.c -o $*.m

.asm.m:
        as $*.asm -o $*.m

.asm.l:
        casm -a $*.asm -cqvy -o $*.l -iwork:cape/includes

.asm.o:
        casm -a $*.asm -cqvy -o $*.o -iwork:cape/includes

libinit.o : inet_rev.h libinit.c
libent.o : inet_rev.i libent.asm

LINETOBJ= lib_custom.l in_cksum.l in_pcb.l in_proto.l ip_input.l ip_output.l \
          raw_ip.l tcp_debug.l tcp_input.l tcp_output.l tcp_subr.l \
          tcp_timer.l tcp_usrreq.l udp_usrreq.l sys_socket.l \
          uipc_domain.l q.l uipc_socket.l uipc_socket2.l \
          uipc_syscalls.l if.l if_loop.l raw_cb.l raw_usrreq.l \
          route.l if_ether.l in.l af.l ip_icmp.l uipc_mbuf.l \
          compat.l proc.l ffs.l inet.l configure.l \
          bmov.l bset.l bcmp.l netproc.l if_s2.l exchange.l


LIBOFILES= libent.o libinit.o if_s2_stubs.o

MINETOBJ= in_cksum.m in_pcb.m in_proto.m ip_input.m ip_output.m \
          raw_ip.m tcp_debug.m tcp_input.m tcp_output.m tcp_subr.m \
          tcp_timer.m tcp_usrreq.m udp_usrreq.m sys_socket.m \
          uipc_domain.m q.m uipc_socket.m uipc_socket2.m \
          uipc_syscalls.m if.m if_loop.m raw_cb.m raw_usrreq.m route.m \
          if_ether.m in.m af.m ip_icmp.m uipc_mbuf.m compat.m proc.m \
          ffs.m inet.m netprocM.m if_ae.m ae_intrM.m \
          bmov.m bset.m bcmp.m

MOBJ=   libstart.m libsup.m doinit.m

all:    inet.library

#all:   inet.library inet.h

inet.library: $(LIBOFILES) $(LINETOBJ) endtag.o
        list >linker.lnk nohead quick #?.l
        blink WITH blink.input MAP inet.map S X F H\
                FROM $(LIBOFILES) WITH linker.lnk \
                LIB lib:lcr.lib lib:lc.lib endtag.o TO $@ SD SC ND BATCH VERBOSE
        copy inet.library inet:libs/inet.library CLONE

inet.library.manx:      $(MOBJ) $(MINETOBJ)
        ln -o $@ $(MOBJ) $(MINETOBJ) -lc

inet.h: inet.fd
        fd2pragma inet.fd inet.h
        copy inet.h //include/proto

clean:
        -delete #?.(l|m|o)
        -delete inet.library.lat
        -delete inet.library

install:        inet.library
        copy inet.library inet:libs clone

bump:
        bumprev 5 inet_rev

release:
        copy #?.(c|h|asm|i) hognet:src/libs/inet.library
        copy makefile blink.input inet.fd inet_rev.rev inet.library hognet:src/libs/inet.library
        copy inet.library hognet:bin/libs/inet.library


